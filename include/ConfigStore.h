#pragma once
#include "Types.h"

class ConfigStore {
public:
  bool begin();
  bool load(PersistentConfig& out);
  bool save(const PersistentConfig& cfg);
  void setDefaults(PersistentConfig& cfg);

private:
  const char* _path = "/config.json";
};
