class StepperConfig {
public:
  StepperConfig(int steps_per_revolution)
      : steps_per_revolution_{steps_per_revolution} {}

  float GetSpeedStepsPerSec(float one_revolution_duration) {
    if (one_revolution_duration < 1.0F) {
      // safe fall back to prevent spin like crazy
      return static_cast<float>(steps_per_revolution_) / 8.0;
    }
    return static_cast<float>(steps_per_revolution_) / one_revolution_duration;
  }

  int GetStepsPerRevolution() const { return steps_per_revolution_; }

private:
  const int steps_per_revolution_;
};
