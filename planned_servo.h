#include "minimum_jerk_trajectory_planner.h"
#include "servo.h"

class PlannedServo : public FeederServo {
public:
  PlannedServo(uint16_t pin, float retreat_time_percentage = 0.2)
      : FeederServo{pin}, retreat_time_percentage_{retreat_time_percentage} {}

  void Plan(float now) {
    now = std::fmod(now, GetIntervalDurationSec());
    float new_position;
    const auto pushing_time = GetPushingTime();
    if (now < pushing_time) {
      new_position = planner_pusher_.Plan(now);
    }
    if (now >= pushing_time) {
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
    interval_duration_sec_ = interval_duration_sec;

    planner_pusher_.Init(GetPushingTime(), end_position);
    planner_retreat_.Init(GetRetreatTime(), end_position);
  }

  float GetIntervalDurationSec() const { return interval_duration_sec_; }

  float GetPushingTime() const {
    return interval_duration_sec_ * (1.0 - retreat_time_percentage_);
  }
  float GetRetreatTime() const {
    return interval_duration_sec_ * (retreat_time_percentage_);
  }

private:
  MinimumJerkTrajortoryPlanner planner_pusher_{};
  MinimumJerkTrajortoryPlanner planner_retreat_{};
  float interval_duration_sec_{};
  float retreat_time_percentage_{};
};
