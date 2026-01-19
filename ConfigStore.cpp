#include "ConfigStore.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <string.h>
#include "Protocol.h"

bool ConfigStore::begin() {
  return LittleFS.begin();
}

void ConfigStore::setDefaults(PersistentConfig& cfg) {
  cfg.ver = 1;
  cfg.wifi.mode = WifiMode::ApSta;
  cfg.wifi.apSubnet = 4;
  cfg.wifi.staSsid[0] = 0;
  cfg.wifi.staPass[0] = 0;

  cfg.rev = {350,150};
  cfg.motion = {350,600};
  cfg.autoRev = {AutoRevMode::Off, 0};

  cfg.rpm = 10;
  cfg.dirFwd = true;
}

bool ConfigStore::load(PersistentConfig& out) {
  if (!LittleFS.exists(_path)) return false;

  File f = LittleFS.open(_path, "r");
  if (!f) return false;

  StaticJsonDocument<1024> doc;
  auto err = deserializeJson(doc, f);
  f.close();
  if (err) return false;

  out.ver = doc["ver"] | 1;

  out.wifi.mode = wifiModeFromStr(doc["wifi"]["mode"] | "apsta");
  out.wifi.apSubnet = doc["wifi"]["apSubnet"] | 4;
  strlcpy(out.wifi.staSsid, doc["wifi"]["staSsid"] | "", sizeof(out.wifi.staSsid));
  strlcpy(out.wifi.staPass, doc["wifi"]["staPass"] | "", sizeof(out.wifi.staPass));

  out.rev.rampMs  = doc["rev"]["rampMs"]  | 350;
  out.rev.pauseMs = doc["rev"]["pauseMs"] | 150;

  out.motion.startRampMs = doc["motion"]["startRampMs"] | 350;
  out.motion.stopRampMs  = doc["motion"]["stopRampMs"]  | 600;

  out.autoRev.mode = autoRevModeFromStr(doc["autoRev"]["mode"] | "off");
  out.autoRev.interval = doc["autoRev"]["interval"] | 0;

  out.rpm = doc["rpm"] | 10;
  out.dirFwd = doc["dirFwd"] | true;

  return true;
}

bool ConfigStore::save(const PersistentConfig& cfg) {
  StaticJsonDocument<1024> doc;

  doc["ver"] = cfg.ver;

  doc["wifi"]["mode"] = wifiModeToStr(cfg.wifi.mode);
  doc["wifi"]["apSubnet"] = cfg.wifi.apSubnet;
  doc["wifi"]["staSsid"] = cfg.wifi.staSsid;
  doc["wifi"]["staPass"] = cfg.wifi.staPass;

  doc["rev"]["rampMs"]  = cfg.rev.rampMs;
  doc["rev"]["pauseMs"] = cfg.rev.pauseMs;

  doc["motion"]["startRampMs"] = cfg.motion.startRampMs;
  doc["motion"]["stopRampMs"]  = cfg.motion.stopRampMs;

  doc["autoRev"]["mode"] = autoRevModeToStr(cfg.autoRev.mode);
  doc["autoRev"]["interval"] = cfg.autoRev.interval;

  doc["rpm"] = cfg.rpm;
  doc["dirFwd"] = cfg.dirFwd;

  File f = LittleFS.open(_path, "w");
  if (!f) return false;
  bool ok = (serializeJson(doc, f) > 0);
  f.close();
  return ok;
}
