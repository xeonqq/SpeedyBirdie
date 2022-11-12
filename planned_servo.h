#include "planner_adapter.h"
#include "servo.h"

class PlannedServo : public FeederServo {
public:
  PlannedServo(uint16_t pin, float push_time, int us_min = 1000,
               int us_max = 2000)
      : FeederServo{pin, us_min, us_max}, push_time_{push_time} {}

  void Plan(float now) {
    now = std::fmod(now, GetIntervalDurationSec());
    float new_position;
    const auto pushing_time = GetPushingTime();
    const auto idle_time = GetIdleTime();
    if (now < idle_time) {
      new_position = planner_pusher_.Plan(0);
    } else if (now < (idle_time + pushing_time)) {
      new_position = planner_pusher_.Plan(now - idle_time);
    } else if (now < GetIntervalDurationSec()) {
      new_position = planner_retreat_.Plan(now - idle_time - pushing_time);
    }
    // Serial.println(new_position);

    FeederServo::Write(new_position);
  }

  void InitByStartAndEnd(float start_value, float end_value) {
    planner_pusher_.InitByStartAndEnd(start_value, end_value);
    planner_retreat_.InitByStartAndEnd(end_value, start_value);
  }

  void InitByDuration(float duration_sec) {
    Init(duration_sec, planner_pusher_.GetStart(), planner_pusher_.GetEnd());
  }

  void Init(float interval_duration_sec, float start_value, float end_value) {
    push_time_ = std::min(interval_duration_sec / 2, push_time_);
    interval_duration_sec_ = interval_duration_sec;

    planner_pusher_.Init(GetPushingTime(), start_value, end_value);
    planner_retreat_.Init(GetRetreatTime(), end_value, start_value);
  }

  float GetIntervalDurationSec() const { return interval_duration_sec_; }

  float GetPushingTime() const { return push_time_; }
  float GetRetreatTime() const { return push_time_; }

  float GetIdleTime() const { return interval_duration_sec_ - 2 * push_time_; }

private:
  PlannerAdapter planner_pusher_{};
  PlannerAdapter planner_retreat_{};
  float interval_duration_sec_{};
  float push_time_{};
};
