#include <Stepper.h>

const int stepsPerRevolution = 2048;

Stepper motor(stepsPerRevolution, 8, 10, 9, 11);

void setup() {
  motor.setSpeed(12);  // RPM
}

void loop() {
  motor.step(2048);   // 360° clockwise
  delay(100);
}
