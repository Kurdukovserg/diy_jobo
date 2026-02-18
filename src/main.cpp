#include <Arduino.h>
#include "App.h"

App app;

void setup() {
  // якщо Serial заважає — не чіпаємо
  delay(50);
  app.begin();
}

void loop() {
  app.tick();
  yield();
}
