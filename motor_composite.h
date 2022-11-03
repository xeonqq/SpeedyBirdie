#include <Servo.h>
class Motors {
public:
  Motors(uint8_t left_motor_pin, uint8_t right_motor_pin) {
    motors_[0].attach(left_motor_pin, min_microseconds_, max_microseconds_);
    motors_[1].attach(right_motor_pin, min_microseconds_, max_microseconds_);
  }

  void Calibrate() {
    ForEach(
        [this](auto &motor) { motor.writeMicroseconds(max_microseconds_); });
    delay(500); // wait 1s
    ForEach(
        [this](auto &motor) { motor.writeMicroseconds(min_microseconds_); });
  }

  void RunSpeed(int power) {
    ForEachI([&](uint16_t i, auto &motor) {
      const int speed_microseconds = map(
          power, 0, 1000, min_microseconds_ + offsets_[i], max_microseconds_);
      motor.writeMicroseconds(speed_microseconds);
    });
  }

private:
  template <typename Func> void ForEachI(Func &&func) {
    for (auto i = 0; i < motors_.size(); ++i) {
      std::forward<Func>(func)(i, motors_[i]);
    }
  }
  template <typename Func> void ForEach(Func &&func) {
    std::for_each(motors_.begin(), motors_.end(), std::forward<Func>(func));
  }

  std::array<Servo, 2> motors_{};
  std::array<uint16_t, 2> offsets_{70, 50};

  const uint16_t max_microseconds_{2000};
  const uint16_t min_microseconds_{1000};
};
