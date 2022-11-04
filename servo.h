
#include "math.h"
#include <Servo.h>

class FeederServo {
public:
  FeederServo(int pin, int us_min = 800, int us_max = 2700)
      : us_min_{us_min}, us_max_{us_max} {
    servo.attach(pin, us_min, us_max);
  }

  void Reset() { servo.writeMicroseconds(us_min_); }

  void Write(float percentage) {
    const auto us_input = math::map(percentage, 0.0F, 1.F, us_min_, us_max_);
    // Serial.print("servo write:");
    // Serial.println(us_input);
    servo.writeMicroseconds(us_input);
  }

private:
  int us_min_;
  int us_max_;
  Servo servo;
};
