#include "brushless_motor.h"
#include <Servo.h>

Servo servo;

BrushlessMotor motor1{D0};
BrushlessMotor motor2{D1};

void setup() { servo.attach(D4); }

void loop() {

  servo.write(90); // set servo to mid-point
  delay(1000);
  servo.write(0); // set servo to mid-point
  delay(1000);
}
