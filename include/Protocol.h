#pragma once
#include <ArduinoJson.h>
#include "Types.h"

const char* stateToStr(ProcState s);
const char* autoRevModeToStr(AutoRevMode m);
AutoRevMode autoRevModeFromStr(const char* s);

const char* wifiModeToStr(WifiMode m);
WifiMode wifiModeFromStr(const char* s);

void buildHello(JsonDocument& doc, const char* fw);
void buildStatus(JsonDocument& doc, const Status& st);
void buildConfig(JsonDocument& doc, const PersistentConfig& cfg);
