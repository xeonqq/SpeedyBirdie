#include "minimum_jerk_trajectory_planner.h"
#include "servo.h"

class PlannedServo : public FeederServo {
public:
  PlannedServo(uint16_t pin, float push_time)
      : FeederServo{pin}, push_time_{push_time} {}

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
      const auto t = (GetIntervalDurationSec() - now);
      // need to go reverse as the planner
      new_position = planner_retreat_.Plan(t);
    }
    FeederServo::Write(new_position);
  }

  void InitByEndPosition(float end_position) {
    planner_pusher_.InitByEndPosition(end_position);
    planner_retreat_.InitByEndPosition(end_position);
  }

  void InitByDuration(float duration_sec) {
    Init(duration_sec, planner_pusher_.GetEndPosition());
  }

  void Init(float interval_duration_sec, float end_position) {
    push_time_ = std::min(interval_duration_sec / 2, push_time_);
    interval_duration_sec_ = interval_duration_sec;

    planner_pusher_.Init(GetPushingTime(), end_position);
    planner_retreat_.Init(GetRetreatTime(), end_position);
  }

  float GetIntervalDurationSec() const { return interval_duration_sec_; }

  float GetPushingTime() const { return push_time_; }
  float GetRetreatTime() const { return push_time_; }

  float GetIdleTime() const { return interval_duration_sec_ - 2 * push_time_; }

private:
  MinimumJerkTrajortoryPlanner planner_pusher_{};
  MinimumJerkTrajortoryPlanner planner_retreat_{};
  float interval_duration_sec_{};
  float push_time_{};
};
