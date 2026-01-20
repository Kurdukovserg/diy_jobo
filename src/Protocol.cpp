#include "Protocol.h"
#include <string.h>

const char* stateToStr(ProcState s){
  switch(s){
    case ProcState::Idle:     return "idle";
    case ProcState::Running:  return "running";
    case ProcState::Paused:   return "paused";
    case ProcState::Stopping: return "stopping";
    default: return "unknown";
  }
}

const char* autoRevModeToStr(AutoRevMode m){
  switch(m){
    case AutoRevMode::Off:   return "off";
    case AutoRevMode::Time:  return "time";
    case AutoRevMode::Turns: return "turns";
    default: return "off";
  }
}

AutoRevMode autoRevModeFromStr(const char* s){
  if(!s) return AutoRevMode::Off;
  if(!strcmp(s,"time"))  return AutoRevMode::Time;
  if(!strcmp(s,"turns")) return AutoRevMode::Turns;
  return AutoRevMode::Off;
}

const char* wifiModeToStr(WifiMode m){
  switch(m){
    case WifiMode::Ap:    return "ap";
    case WifiMode::Sta:   return "sta";
    case WifiMode::ApSta: return "apsta";
    default: return "apsta";
  }
}

WifiMode wifiModeFromStr(const char* s){
  if(!s) return WifiMode::ApSta;
  if(!strcmp(s,"ap"))    return WifiMode::Ap;
  if(!strcmp(s,"sta"))   return WifiMode::Sta;
  return WifiMode::ApSta;
}

void buildHello(JsonDocument& doc, const char* fw){
  doc["type"]="hello";
  doc["device"]="DIY-JOBO";
  doc["fw"]=fw;
}

void buildStatus(JsonDocument& doc, const Status& st){
  doc["type"]="status";
  doc["state"]=stateToStr(st.state);
  doc["rpm"]=st.rpm;
  doc["dir"]=st.dirFwd ? "fwd":"rev";
  doc["rev"]=st.reversing;

  doc["rev_ramp_ms"]=st.rev.rampMs;
  doc["rev_pause_ms"]=st.rev.pauseMs;
  doc["start_ramp_ms"]=st.motion.startRampMs;
  doc["stop_ramp_ms"]=st.motion.stopRampMs;

  doc["auto_rev_mode"]=autoRevModeToStr(st.autoRev.mode);
  doc["auto_rev_interval"]=st.autoRev.interval;

  doc["temp"]=st.tempC;
}

void buildConfig(JsonDocument& doc, const PersistentConfig& cfg){
  doc["type"]="config";

  doc["wifi"]["mode"]=wifiModeToStr(cfg.wifi.mode);
  doc["wifi"]["staSsid"]=cfg.wifi.staSsid;
  doc["wifi"]["apSubnet"]=cfg.wifi.apSubnet;

  doc["rpm"]=cfg.rpm;
  doc["dirFwd"]=cfg.dirFwd;

  doc["rev"]["rampMs"]=cfg.rev.rampMs;
  doc["rev"]["pauseMs"]=cfg.rev.pauseMs;

  doc["motion"]["startRampMs"]=cfg.motion.startRampMs;
  doc["motion"]["stopRampMs"]=cfg.motion.stopRampMs;

  doc["autoRev"]["mode"]=autoRevModeToStr(cfg.autoRev.mode);
  doc["autoRev"]["interval"]=cfg.autoRev.interval;
}
