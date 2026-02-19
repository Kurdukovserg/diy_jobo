#include <Arduino.h>
#include "App.h"

App app;

void setup() {
  delay(50);
  app.begin();
}

void loop() {
  app.tick();
  yield();
}
