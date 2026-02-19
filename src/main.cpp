#include <Arduino.h>
#include "App.h"

App app;

void setup() {
  Serial.begin(115200);
  delay(50);
  app.begin();
  Serial.println("Setup complete");
}

void loop() {
  app.tick();
  yield();
}
