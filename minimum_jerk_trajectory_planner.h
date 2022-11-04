#include <algorithm>
class MinimumJerkTrajortoryPlanner {
public:
  MinimumJerkTrajortoryPlanner(float duration_sec = 5,
                               float end_position = 500) {
    Init(duration_sec, end_position);
  }
  void Init(float duration_sec, float end_position) {
    duration_sec_ = duration_sec;
    const auto T = duration_sec;
    const auto T_square = T * T;
    const auto T_power_3 = T_square * T;
    const auto T_power_4 = T_power_3 * T;
    const auto T_power_5 = T_power_4 * T;

    c3 = 10 * end_position / T_power_3;
    c4 = -15 * end_position / T_power_4;
    c5 = 6 * end_position / T_power_5;
  }

  float Plan(float t) const {
    t = std::clamp(t, 0.0F, duration_sec_);
    const auto t_power_2 = t * t;
    const auto t_power_3 = t * t_power_2;
    return t_power_3 * (c3 + c4 * t + c5 * t_power_2);
  }
  float GetDuration() const { return duration_sec_; }

private:
  float duration_sec_;
  float c3, c4, c5;
};