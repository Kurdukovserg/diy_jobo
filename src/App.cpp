#include "App.h"
#include "Config.h"

// сюди потім підключиш u8g2, temp, motor, ws і т.д.

void App::begin() {
  _in.begin();
}

void App::tick() {
  InputsSnapshot s = _in.tick();
  handleInputs(s);

  // тут буде: motor.tick(); temp.tick(); ui.tick(); ws.tick();
}

void App::handleInputs(const InputsSnapshot& s) {
  if (s.encDelta != 0) {
    _rpm += s.encDelta;
    clampRpm();
  }

  // Confirm/OK: можемо приймати або кнопку OK, або SW, або RX-confirm
  if (s.okPressed || s.encSwPressed) {
    _run = !_run;
    // motor.setRun(_run);
  }

  // Back: приймаємо або GPIO back, або A0 back
  if (s.backPressed || s.a0BackPressed) {
    _run = false;
    // motor.setRun(false);
    // motor.setTargetRpm(0);
  }

  // target rpm: тільки якщо run
  // motor.setTargetRpm(_run ? _rpm : 0);
}

void App::clampRpm() {
  if (_rpm < RPM_MIN) _rpm = RPM_MIN;
  if (_rpm > RPM_MAX) _rpm = RPM_MAX;
}
