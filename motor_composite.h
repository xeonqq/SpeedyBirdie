#include <Servo.h>
class Motors {
public:
  Motors(uint8_t left_motor_pin, uint8_t right_motor_pin, uint16_t us_min,
         uint16_t us_max)
      : min_microseconds_{us_min}, max_microseconds_{us_max} {
    // initialize stored last value, before attach
    ForEach([us_min](auto &motor) { motor.writeMicroseconds(us_min); });
    motors_[0].attach(left_motor_pin, min_microseconds_, max_microseconds_);
    motors_[1].attach(right_motor_pin, min_microseconds_, max_microseconds_);
  }

  void Stop() {
    ForEach(
        [this](auto &motor) { motor.writeMicroseconds(min_microseconds_); });
  }

  void Write(int power) {
    ForEachI([&](uint16_t i, auto &motor) {
      const int speed_microseconds = map(
          power, 0, 1000, min_microseconds_ + offsets_[i], max_microseconds_);
      motor.writeMicroseconds(ConstrainPWM(speed_microseconds));
    });
  }

  void WriteRaw(uint16_t motor_index, int power) {
    const int speed_microseconds =
        map(power, 0, 1000, min_microseconds_, max_microseconds_);
    motors_[motor_index].writeMicroseconds(ConstrainPWM(speed_microseconds));
  }

  void SetPwmOffsets(const std::array<uint16_t, 2> &offsets) {
    offsets_ = offsets;
  }

  int Read() { return motors_[0].readMicroseconds() - min_microseconds_; }

private:
  int ConstrainPWM(int value) {
    return constrain(value, min_microseconds_, 1300);
  }

  template <typename Func> void ForEachI(Func &&func) {
    for (auto i = 0; i < motors_.size(); ++i) {
      std::forward<Func>(func)(i, motors_[i]);
    }
  }
  template <typename Func> void ForEach(Func &&func) {
    std::for_each(motors_.begin(), motors_.end(), std::forward<Func>(func));
  }

  std::array<Servo, 2> motors_{};
  std::array<uint16_t, 2> offsets_{};

  const uint16_t max_microseconds_{2000};
  const uint16_t min_microseconds_{1000};
};
