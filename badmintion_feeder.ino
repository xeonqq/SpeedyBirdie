#include "brushless_motor.h"
#include "config.h"
#include <AccelStepper.h>
#include <Servo.h>

#define IN1 1
#define IN2 3
#define IN3 15
#define IN4 13

StepperConfig stepper_config{200};
BadmintonFeederConfig badmintion_feeder_config{5};

Servo servo;

AccelStepper stepper(AccelStepper::HALF4WIRE, IN1, IN2, IN3, IN4);
BrushlessMotor motor1{D0};
BrushlessMotor motor2{D1};

void setup() {
  motor1.Calibrate();
  motor2.Calibrate();

  Serial.begin(115200);
  servo.attach(D4);
  // set the speed and acceleration
  const auto speed = stepper_config.GetSpeedStepsPerSec(
      badmintion_feeder_config.GetShootIntervalSec());
  stepper.setMaxSpeed(speed * 2);
  stepper.setSpeed(speed);
  stepper.setAcceleration(200);
}

void loop() {

  stepper.move(stepper_config.GetStepsPerRevolution() * 4);
  stepper.run();

  motor1.RunSpeed(0.0);
}
