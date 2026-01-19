#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <math.h>

#include "Config.h"
#include "Types.h"
#include "Protocol.h"
#include "WsServer.h"
#include "MotorController.h"
#include "AutoReverse.h"
#include "EncoderInput.h"
#include "DiscoveryUdp.h"
#include "ConfigStore.h"
#include "TempSensor.h"
#include "Display.h"

// ---------- Globals ----------
static const char* FW_VER = "0.11-wledlike";

WebSocketsServer wsRaw(WS_PORT);
WsServer ws(wsRaw);

MotorController motor;

AutoReverse autoRev;
EncoderInput enc;
DiscoveryUdp discovery;
ConfigStore store;
TempSensor tempSensor;
Display display;

PersistentConfig cfg;
Status st;

unsigned long lastStatusMs = 0;
unsigned long lastCfgSaveMs = 0;
bool cfgDirty = false;

// ---------- WiFi helpers ----------
static void startAp(uint8_t subnet) {
  IPAddress apIP(192,168,subnet,1);
  IPAddress gw(192,168,subnet,1);
  IPAddress mask(255,255,255,0);

  WiFi.softAPConfig(apIP, gw, mask);
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS);
}

static bool trySta(const char* ssid, const char* pass, uint32_t timeoutMs=12000) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  uint32_t t0 = millis();
  while (millis() - t0 < timeoutMs) {
    if (WiFi.status() == WL_CONNECTED) return true;
    delay(100);
  }
  return false;
}

static void setupWiFiFromConfig(const PersistentConfig& c) {
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);

  bool wantSta = (c.wifi.mode == WifiMode::Sta || c.wifi.mode == WifiMode::ApSta) && c.wifi.staSsid[0] != 0;

  if (wantSta) {
    bool ok = trySta(c.wifi.staSsid, c.wifi.staPass);
    if (ok) {
      if (c.wifi.mode == WifiMode::ApSta) {
        WiFi.mode(WIFI_AP_STA);
        startAp(c.wifi.apSubnet);
      }
      return;
    }
  }

  // fallback AP
  WiFi.mode(WIFI_AP);
  startAp(c.wifi.apSubnet);
}

// ---------- status <-> cfg sync ----------
static void applyRuntimeFromCfg() {
  st.rpm = constrain(cfg.rpm, RPM_MIN, RPM_MAX);
  st.dirFwd = cfg.dirFwd;

  st.rev = cfg.rev;
  st.motion = cfg.motion;
  st.autoRev = cfg.autoRev;

  // motor apply
  motor.setRpm(st.rpm);
  motor.setDir(st.dirFwd);
  motor.setRevParams(st.rev);
  motor.setMotionParams(st.motion);

  autoRev.setConfig(st.autoRev);
}

static void persistCfg() {
  // Mark dirty, actual save is debounced in loop
  cfgDirty = true;
}

static void debouncedSaveCfg() {
  if (!cfgDirty) return;
  unsigned long now = millis();
  if (now - lastCfgSaveMs < 2000) return; // save at most every 2 sec
  lastCfgSaveMs = now;
  cfgDirty = false;
  store.save(cfg);
}

// ---------- WS send helpers ----------
static void sendHello() {
  StaticJsonDocument<128> doc;
  buildHello(doc, FW_VER);
  ws.broadcastDoc(doc);
}

static void sendStatus() {
  st.reversing = motor.isReversing();
  st.dirFwd = motor.dirFwd();
  StaticJsonDocument<256> doc;
  buildStatus(doc, st);
  ws.broadcastDoc(doc);
}

static void sendConfig() {
  StaticJsonDocument<512> doc;
  buildConfig(doc, cfg);
  ws.broadcastDoc(doc);
}

static void sendRestarting() {
  StaticJsonDocument<128> doc;
  doc["type"] = "system";
  doc["status"] = "restarting";
  ws.broadcastDoc(doc);
}

// ---------- Commands ----------
static void handleCommand(const JsonDocument& doc) {
  const char* cmd = doc["cmd"];
  if (!cmd) return;

  if (!strcmp(cmd, "start")) {
    st.state = ProcState::Running;
    motor.setRunning(true);
    applyRuntimeFromCfg();
    motor.start();
    sendStatus();
    return;
  }

  if (!strcmp(cmd, "stop")) {
    motor.stop();
    motor.setRunning(false);
    st.state = ProcState::Stopping;
    sendStatus();
    return;
  }

  if (!strcmp(cmd, "pause")) {
    motor.stop();
    motor.setRunning(false);
    st.state = ProcState::Paused;
    sendStatus();
    return;
  }

  if (!strcmp(cmd, "reverse")) {
    motor.requestReverse();
    cfg.dirFwd = motor.dirFwd();
    persistCfg();
    sendStatus();
    return;
  }

  if (!strcmp(cmd, "set_rpm")) {
    int v = doc["value"] | cfg.rpm;
    cfg.rpm = constrain(v, RPM_MIN, RPM_MAX);
    applyRuntimeFromCfg();
    persistCfg();
    sendConfig();
    sendStatus();
    return;
  }

  if (!strcmp(cmd, "set_rev_params")) {
    uint16_t r = doc["ramp_ms"]  | cfg.rev.rampMs;
    uint16_t p = doc["pause_ms"] | cfg.rev.pauseMs;
    cfg.rev.rampMs  = constrain((int)r, 50, 2000);
    cfg.rev.pauseMs = constrain((int)p, 0, 2000);
    applyRuntimeFromCfg();
    persistCfg();
    sendConfig();
    sendStatus();
    return;
  }

  if (!strcmp(cmd, "set_motion_params")) {
    uint16_t sr = doc["start_ramp_ms"] | cfg.motion.startRampMs;
    uint16_t er = doc["stop_ramp_ms"]  | cfg.motion.stopRampMs;
    cfg.motion.startRampMs = constrain((int)sr, 50, 5000);
    cfg.motion.stopRampMs  = constrain((int)er, 50, 5000);
    applyRuntimeFromCfg();
    persistCfg();
    sendConfig();
    sendStatus();
    return;
  }

  if (!strcmp(cmd, "set_auto_reverse")) {
    cfg.autoRev.mode = autoRevModeFromStr(doc["mode"] | "off");
    cfg.autoRev.interval = doc["interval"] | cfg.autoRev.interval;
    applyRuntimeFromCfg();
    autoRev.reset();
    persistCfg();
    sendConfig();
    sendStatus();
    return;
  }

  if (!strcmp(cmd, "get_config")) {
    sendConfig();
    return;
  }

  // ---- WiFi (WLED-like) ----
  // {"cmd":"set_wifi","mode":"apsta","staSsid":"MyWiFi","staPass":"pass","apSubnet":4}
  if (!strcmp(cmd, "set_wifi")) {
    cfg.wifi.mode = wifiModeFromStr(doc["mode"] | "apsta");
    cfg.wifi.apSubnet = doc["apSubnet"] | cfg.wifi.apSubnet;

    const char* ssid = doc["staSsid"] | cfg.wifi.staSsid;
    const char* pass = doc["staPass"] | cfg.wifi.staPass;

    strlcpy(cfg.wifi.staSsid, ssid, sizeof(cfg.wifi.staSsid));
    strlcpy(cfg.wifi.staPass, pass, sizeof(cfg.wifi.staPass));

    persistCfg();
    sendConfig();
    sendRestarting();
    delay(200);
    ESP.restart();
    return;
  }
}

// ---------- WS callback ----------
static void onWsEvent(uint8_t, WStype_t type, uint8_t* payload, size_t) {
  if (type != WStype_TEXT) return;

  StaticJsonDocument<768> doc;
  if (deserializeJson(doc, payload)) return;

  handleCommand(doc);
}


// ---------- Helper ----------
static float rpmToSps(int rpm) {
  return (rpm * (float)STEPS_EFF) / 60.0f;
}

// ---------- Setup / Loop ----------
void setup() {
  Serial.begin(115200);
  delay(50);

  // FS + config
  store.begin();
  if (!store.load(cfg)) {
    store.setDefaults(cfg);
    store.save(cfg);
  }

  // runtime init
  st.state = ProcState::Idle;
  st.tempC = 0.0f;

  // wifi
  setupWiFiFromConfig(cfg);

  // motor init (NEMA17 + TMC2209) - uses timer interrupt for stepping
  motor.begin(PIN_STEP, PIN_DIR, cfg.rpm, cfg.dirFwd, cfg.rev, cfg.motion);
  motor.setRunning(false);

  // encoder + buttons
  enc.begin(PIN_ENC_A, PIN_ENC_B, PIN_BTN_OK, PIN_BTN_BACK);

  // temperature sensor (DS18B20)
  tempSensor.begin(PIN_DS18B20);

  // OLED display
  display.begin(PIN_SDA, PIN_SCL);

  // apply runtime from config
  applyRuntimeFromCfg();
  autoRev.reset();

  // ws
  ws.begin(onWsEvent);

  // discovery (works in AP and STA)
  discovery.begin(DISCOVERY_PORT, DEVICE_NAME, HTTP_PORT, WS_PORT, FW_VER);
  discovery.setIntervalMs(1000);

  sendHello();
  sendConfig();
  sendStatus();

  Serial.println("DIY-JOBO started (NEMA17 + TMC2209 + OLED + DS18B20)");
}

void loop() {
  // Stepping is now handled by timer interrupt - no need for motor.run() calls!
  
  // Network
  ws.loop();
  discovery.tick();

  // encoder + buttons
  enc.tick();

  // rpm change via encoder rotation
  if (enc.rpmChanged()) {
    int d = enc.consumeRpmDelta();
    cfg.rpm = constrain(cfg.rpm + d, RPM_MIN, RPM_MAX);
    Serial.printf("RPM: %d (delta: %d)\n", cfg.rpm, d);
    applyRuntimeFromCfg();
    persistCfg();  // debounced
  }

  // OK button -> start/stop toggle
  if (enc.okPressed()) {
    if (st.state == ProcState::Running) {
      motor.stop();
      motor.setRunning(false);
      st.state = ProcState::Stopping;
    } else {
      st.state = ProcState::Running;
      motor.setRunning(true);
      applyRuntimeFromCfg();
      motor.start();
    }
    sendStatus();
  }

  // BACK button -> stop / reverse when stopped
  if (enc.backPressed()) {
    if (st.state == ProcState::Running) {
      motor.stop();
      motor.setRunning(false);
      st.state = ProcState::Stopping;
    } else {
      motor.requestReverse();
      cfg.dirFwd = motor.dirFwd();
      persistCfg();
    }
    sendStatus();
  }

  // motor FSM tick (updates speed for ramps/reverse)
  motor.tick();

  // auto reverse
  autoRev.tick(
    motor.currentSps(),
    (st.state == ProcState::Running),
    [](){
      motor.requestReverse();
      cfg.dirFwd = motor.dirFwd();
      persistCfg();
      sendStatus();
    }
  );

  // stopping -> idle when finished
  if (st.state == ProcState::Stopping && motor.isStopped()) {
    st.state = ProcState::Idle;
    sendStatus();
  }

  // temperature sensor
  tempSensor.tick();
  if (tempSensor.hasReading()) {
    st.tempC = tempSensor.tempC();
  }

  // OLED display update (always updates now - stepping is interrupt-driven)
  float currentRpm = (motor.currentSps() * 60.0f) / (float)STEPS_EFF;
  bool motorActive = motor.shouldRun();
  display.tick(st, fabsf(currentRpm), (float)cfg.rpm, st.tempC, tempSensor.hasReading(), motorActive);

  // periodic WS status broadcast
  unsigned long now = millis();
  if (now - lastStatusMs > 1000) {
    lastStatusMs = now;
    sendStatus();
  }

  // debounced config save
  debouncedSaveCfg();

  yield();
}
