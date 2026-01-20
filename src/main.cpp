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
UIScreen uiScreen = UIScreen::Main;
int menuIndex = 0;
bool editMode = false;  // true when editing a value

// Menu item counts
const int MENU_MAIN_ITEMS = 5;      // Auto-Rev, Accel, Temp, Process, WiFi
const int MENU_AUTOREV_ITEMS = 3;   // Mode, Interval, Back
const int MENU_ACCEL_ITEMS = 3;     // Start, Stop, Back
const int MENU_TEMP_ITEMS = 4;      // Enable, Base, Coeff, Alarm
const int MENU_PROCESS_ITEMS = 2;   // Time, Back

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

  // encoder + buttons (including encoder SW for menu)
  enc.begin(PIN_ENC_A, PIN_ENC_B, PIN_ENC_SW, PIN_BTN_OK, PIN_BTN_BACK);

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

  // Encoder rotation handling
  if (enc.rpmChanged()) {
    int d = enc.consumeRpmDelta();
    
    if (uiScreen == UIScreen::Main) {
      // Main screen: adjust RPM
      cfg.rpm = constrain(cfg.rpm + d, RPM_MIN, RPM_MAX);
      applyRuntimeFromCfg();
      persistCfg();
    } else if (uiScreen == UIScreen::Menu) {
      // Main menu navigation
      menuIndex = (menuIndex + d + MENU_MAIN_ITEMS) % MENU_MAIN_ITEMS;
    } else if (uiScreen == UIScreen::AutoRev) {
      // Auto-reverse submenu navigation
      menuIndex = (menuIndex + d + MENU_AUTOREV_ITEMS) % MENU_AUTOREV_ITEMS;
    } else if (uiScreen == UIScreen::AutoRevEdit) {
      // Editing auto-reverse values
      if (menuIndex == 0) {
        // Cycle mode: Off -> Time -> Turns -> Off
        int m = (int)cfg.autoRev.mode + d;
        cfg.autoRev.mode = (AutoRevMode)((m + 3) % 3);
      } else if (menuIndex == 1) {
        // Adjust interval
        cfg.autoRev.interval = constrain((int)cfg.autoRev.interval + d * 5, 0, 600);
      }
      applyRuntimeFromCfg();
      persistCfg();
    } else if (uiScreen == UIScreen::Accel) {
      // Acceleration submenu navigation
      menuIndex = (menuIndex + d + MENU_ACCEL_ITEMS) % MENU_ACCEL_ITEMS;
    } else if (uiScreen == UIScreen::AccelEdit) {
      // Editing acceleration values
      int step = 50;
      if (menuIndex == 0) {
        cfg.motion.startRampMs = constrain((int)cfg.motion.startRampMs + d * step, 50, 3000);
      } else if (menuIndex == 1) {
        cfg.motion.stopRampMs = constrain((int)cfg.motion.stopRampMs + d * step, 50, 3000);
      }
      applyRuntimeFromCfg();
      persistCfg();
    } else if (uiScreen == UIScreen::TempMenu) {
      // Temperature submenu navigation
      menuIndex = (menuIndex + d + MENU_TEMP_ITEMS) % MENU_TEMP_ITEMS;
    } else if (uiScreen == UIScreen::TempEdit) {
      // Editing temperature values
      if (menuIndex == 0) {
        // Toggle enabled
        cfg.temp.enabled = !cfg.temp.enabled;
      } else if (menuIndex == 1) {
        // Adjust base temp (0.5 degree steps)
        cfg.temp.baseTemp = constrain(cfg.temp.baseTemp + d * 0.5f, 15.0f, 40.0f);
      } else if (menuIndex == 2) {
        // Adjust coefficient (0.05 steps)
        cfg.temp.coefficient = constrain(cfg.temp.coefficient + d * 0.05f, 0.5f, 2.0f);
      } else if (menuIndex == 3) {
        // Adjust alarm threshold (0.5 degree steps)
        cfg.temp.alarmThreshold = constrain(cfg.temp.alarmThreshold + d * 0.5f, 1.0f, 10.0f);
      }
      persistCfg();
    } else if (uiScreen == UIScreen::ProcessMenu) {
      // Process submenu navigation
      menuIndex = (menuIndex + d + MENU_PROCESS_ITEMS) % MENU_PROCESS_ITEMS;
    } else if (uiScreen == UIScreen::ProcessEdit) {
      // Editing process time (30 sec steps)
      int newTime = (int)cfg.processTimeSec + d * 30;
      cfg.processTimeSec = constrain(newTime, 30, 3600);  // 30s to 60min
      persistCfg();
    }
  }

  // BACK button -> exit edit / go back in menu / stop motor / reverse
  if (enc.backPressed()) {
    if (uiScreen == UIScreen::AutoRevEdit || uiScreen == UIScreen::AccelEdit || uiScreen == UIScreen::TempEdit || uiScreen == UIScreen::ProcessEdit) {
      // Exit edit mode, stay in submenu
      if (uiScreen == UIScreen::AutoRevEdit) uiScreen = UIScreen::AutoRev;
      else if (uiScreen == UIScreen::AccelEdit) uiScreen = UIScreen::Accel;
      else if (uiScreen == UIScreen::TempEdit) uiScreen = UIScreen::TempMenu;
      else uiScreen = UIScreen::ProcessMenu;
    } else if (uiScreen == UIScreen::AutoRev || uiScreen == UIScreen::Accel || uiScreen == UIScreen::TempMenu || uiScreen == UIScreen::ProcessMenu || uiScreen == UIScreen::WiFiMenu) {
      // Back to main menu
      uiScreen = UIScreen::Menu;
      menuIndex = 0;
    } else if (uiScreen == UIScreen::Menu) {
      // Back to main screen
      uiScreen = UIScreen::Main;
      menuIndex = 0;
    } else if (uiScreen == UIScreen::StopConfirm) {
      // Cancel stop - resume
      uiScreen = UIScreen::Main;
      if (st.state == ProcState::Paused) {
        st.state = ProcState::Running;
        motor.setRunning(true);
        motor.start();
      }
    } else if (st.state == ProcState::Running || st.state == ProcState::Paused) {
      // Show stop confirmation
      uiScreen = UIScreen::StopConfirm;
      menuIndex = 0;  // 0 = Cancel, 1 = Stop
    } else {
      // Idle - reverse direction
      motor.requestReverse();
      cfg.dirFwd = motor.dirFwd();
      persistCfg();
    }
    sendStatus();
  }

  // OK button (D6 - encoder click) -> start/pause/resume on main, select in menus
  if (enc.okPressed()) {
    if (uiScreen == UIScreen::Main && st.state == ProcState::Idle) {
      // Start motor
      st.state = ProcState::Running;
      motor.setRunning(true);
      applyRuntimeFromCfg();
      motor.start();
      
      // Start timer with temperature-adjusted duration
      if (cfg.processTimeSec > 0) {
        float factor = 1.0f;
        if (cfg.temp.enabled && tempSensor.hasReading()) {
          float delta = st.tempC - cfg.temp.baseTemp;
          factor = 1.0f - delta * cfg.temp.coefficient * 0.01f;
          if (factor < 0.5f) factor = 0.5f;
          if (factor > 2.0f) factor = 2.0f;
        }
        st.timerDurationSec = (uint16_t)(cfg.processTimeSec * factor);
        st.timerStartMs = millis();
        st.timerActive = true;
      }
      sendStatus();
    } else if (uiScreen == UIScreen::Main && (st.state == ProcState::Running || st.state == ProcState::Paused)) {
      // Pause/Resume toggle
      if (st.state == ProcState::Running) {
        st.state = ProcState::Paused;
        motor.setRunning(false);
        motor.stop();
        // Save elapsed time when pausing
        if (st.timerActive) {
          st.timerPausedSec = (millis() - st.timerStartMs) / 1000;
        }
      } else {
        st.state = ProcState::Running;
        motor.setRunning(true);
        motor.start();
        // Restore timer from paused state
        if (st.timerActive) {
          st.timerStartMs = millis() - (st.timerPausedSec * 1000);
        }
      }
      sendStatus();
    } else if (uiScreen == UIScreen::Menu) {
      // Select submenu (same as confirmPressed)
      if (menuIndex == 0) { uiScreen = UIScreen::AutoRev; menuIndex = 0; }
      else if (menuIndex == 1) { uiScreen = UIScreen::Accel; menuIndex = 0; }
      else if (menuIndex == 2) { uiScreen = UIScreen::TempMenu; menuIndex = 0; }
      else if (menuIndex == 3) { uiScreen = UIScreen::ProcessMenu; menuIndex = 0; }
      else if (menuIndex == 4) { uiScreen = UIScreen::WiFiMenu; }
    } else if (uiScreen == UIScreen::AutoRev) {
      if (menuIndex == 2) { uiScreen = UIScreen::Menu; menuIndex = 0; }
      else { uiScreen = UIScreen::AutoRevEdit; }
    } else if (uiScreen == UIScreen::AutoRevEdit) {
      uiScreen = UIScreen::AutoRev;
    } else if (uiScreen == UIScreen::Accel) {
      if (menuIndex == 2) { uiScreen = UIScreen::Menu; menuIndex = 1; }
      else { uiScreen = UIScreen::AccelEdit; }
    } else if (uiScreen == UIScreen::AccelEdit) {
      uiScreen = UIScreen::Accel;
    } else if (uiScreen == UIScreen::TempMenu) {
      uiScreen = UIScreen::TempEdit;
    } else if (uiScreen == UIScreen::TempEdit) {
      uiScreen = UIScreen::TempMenu;
    } else if (uiScreen == UIScreen::ProcessMenu) {
      if (menuIndex == 1) { uiScreen = UIScreen::Menu; menuIndex = 3; }
      else { uiScreen = UIScreen::ProcessEdit; }
    } else if (uiScreen == UIScreen::ProcessEdit) {
      uiScreen = UIScreen::ProcessMenu;
    } else if (uiScreen == UIScreen::WiFiMenu) {
      uiScreen = UIScreen::Menu; menuIndex = 4;
    }
  }

  // Confirm button (TX) -> stop confirmed, enter menu on main screen, also select in menus
  if (enc.confirmPressed()) {
    if (uiScreen == UIScreen::StopConfirm) {
      // TX = Stop confirmed
      motor.stop();
      motor.setRunning(false);
      st.state = ProcState::Stopping;
      st.timerActive = false;
      uiScreen = UIScreen::Main;
      sendStatus();
    } else if (uiScreen == UIScreen::Main) {
      // Enter menu (works in any state)
      uiScreen = UIScreen::Menu;
      menuIndex = 0;
    } else if (uiScreen == UIScreen::Menu) {
      // Select submenu
      if (menuIndex == 0) { uiScreen = UIScreen::AutoRev; menuIndex = 0; }
      else if (menuIndex == 1) { uiScreen = UIScreen::Accel; menuIndex = 0; }
      else if (menuIndex == 2) { uiScreen = UIScreen::TempMenu; menuIndex = 0; }
      else if (menuIndex == 3) { uiScreen = UIScreen::ProcessMenu; menuIndex = 0; }
      else if (menuIndex == 4) { uiScreen = UIScreen::WiFiMenu; }
    } else if (uiScreen == UIScreen::AutoRev) {
      if (menuIndex == 2) {
        // Back selected
        uiScreen = UIScreen::Menu;
        menuIndex = 0;
      } else {
        // Enter edit mode
        uiScreen = UIScreen::AutoRevEdit;
      }
    } else if (uiScreen == UIScreen::AutoRevEdit) {
      // Exit edit mode
      uiScreen = UIScreen::AutoRev;
    } else if (uiScreen == UIScreen::Accel) {
      if (menuIndex == 2) {
        uiScreen = UIScreen::Menu;
        menuIndex = 1;
      } else {
        uiScreen = UIScreen::AccelEdit;
      }
    } else if (uiScreen == UIScreen::AccelEdit) {
      uiScreen = UIScreen::Accel;
    } else if (uiScreen == UIScreen::TempMenu) {
      // Enter edit mode for temperature
      uiScreen = UIScreen::TempEdit;
    } else if (uiScreen == UIScreen::TempEdit) {
      uiScreen = UIScreen::TempMenu;
    } else if (uiScreen == UIScreen::ProcessMenu) {
      if (menuIndex == 1) {
        // Back selected
        uiScreen = UIScreen::Menu;
        menuIndex = 3;
      } else {
        uiScreen = UIScreen::ProcessEdit;
      }
    } else if (uiScreen == UIScreen::ProcessEdit) {
      uiScreen = UIScreen::ProcessMenu;
    } else if (uiScreen == UIScreen::WiFiMenu) {
      // Back from WiFi info
      uiScreen = UIScreen::Menu;
      menuIndex = 4;
    }
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

  // Timer completion check - auto-stop when timer expires
  if (st.timerActive && st.state == ProcState::Running) {
    uint32_t elapsedSec = (millis() - st.timerStartMs) / 1000;
    if (elapsedSec >= st.timerDurationSec) {
      motor.stop();
      motor.setRunning(false);
      st.state = ProcState::Stopping;
      st.timerActive = false;
      sendStatus();
    }
  }

  // stopping -> idle when finished
  if (st.state == ProcState::Stopping && motor.isStopped()) {
    st.state = ProcState::Idle;
    sendStatus();
  }

  // temperature sensor
  tempSensor.tick();
  if (tempSensor.hasReading()) {
    st.tempC = tempSensor.tempC();
    
    // Check temperature alarm (only when running and temp compensation enabled)
    if (cfg.temp.enabled && st.state == ProcState::Running) {
      float deviation = fabsf(st.tempC - cfg.temp.baseTemp);
      st.tempAlarm = (deviation > cfg.temp.alarmThreshold);
    } else {
      st.tempAlarm = false;
    }
  }

  // OLED display update
  float currentRpm = (motor.currentSps() * 60.0f) / (float)STEPS_EFF;
  bool motorActive = motor.shouldRun();
  bool isEditing = (uiScreen == UIScreen::AutoRevEdit || uiScreen == UIScreen::AccelEdit || 
                     uiScreen == UIScreen::TempEdit || uiScreen == UIScreen::ProcessEdit);
  display.tick(st, cfg, fabsf(currentRpm), st.tempC, tempSensor.hasReading(), motorActive, uiScreen, menuIndex, isEditing);

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
