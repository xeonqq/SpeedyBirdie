#ifndef INCLUDE_PS_H_
#define INCLUDE_PS_H_
#include "planner_adapter.h"
#include <SmingCore.h>

template <typename ServoStrategy> class PlannedServo : public ServoStrategy
{
public:
	template <typename... Args>
	PlannedServo(float push_time, Args... args) : ServoStrategy{args...}, push_time_{push_time}
	{
	}

	void Plan(float now)
	{
		if(now < 0) {
			return;
		}

		//Serial << "now:" << now;
		now = std::fmod(now, GetIntervalDurationSec());
		//Serial << " interval:" << GetIntervalDurationSec() << " after fmod:" << now;
		//Serial.println();
		float new_position{};
		const auto pushing_time = GetPushingTime();
		const auto idle_time = GetIdleTime();
		if(now < idle_time) {
			new_position = planner_pusher_.Plan(0);
			//Serial << "idle\n";
		} else if(now < (idle_time + pushing_time)) {
			new_position = planner_pusher_.Plan(now - idle_time);
			//Serial << "pushing @" << now - idle_time;
		} else //if(now < GetIntervalDurationSec())
		{
			new_position = planner_retreat_.Plan(now - idle_time - pushing_time);
			//Serial << "retreat @" << now - idle_time - pushing_time;
		}
		//Serial.println();
		//Serial.println(new_position);

		ServoStrategy::Write(new_position);
	}

	void InitByStartAndEnd(float start_value, float end_value)
	{
		planner_pusher_.InitByStartAndEnd(start_value, end_value);
		planner_retreat_.InitByStartAndEnd(end_value, start_value);
		InitSmoothen();
	}

	void InitByDuration(float duration_sec)
	{
		Init(duration_sec, planner_pusher_.GetStart(), planner_pusher_.GetEnd());
	}

	void Init(float interval_duration_sec, float start_value, float end_value, bool smoothen = true)
	{
		push_time_ = std::min(interval_duration_sec / 2, push_time_);
		interval_duration_sec_ = interval_duration_sec;

		planner_pusher_.Init(GetPushingTime(), start_value, end_value);
		planner_retreat_.Init(GetRetreatTime(), end_value, start_value);
		if(smoothen) {
			InitSmoothen();
		}
	}

	float GetIntervalDurationSec() const
	{
		return interval_duration_sec_;
	}

	float GetPushingTime() const
	{
		return push_time_;
	}
	float GetRetreatTime() const
	{
		return push_time_;
	}

	float GetIdleTime() const
	{
		return interval_duration_sec_ - 2 * push_time_;
	}

	bool IsReady() const
	{
		return !smoothen_timer_.isStarted();
	}
	void InitSmoothen()
	{
		float start = ServoStrategy::Read();
		float end = planner_pusher_.GetStart();
		if(start == end) {
			return;
		}
		float duration = 2.F;
		planner_smoothen_.Init(duration, start, end);
		smoothen_t_ = 0.F;
		smoothen_timer_
			.initializeMs(static_cast<int>(smoothen_dt_ * 1000),
						  [&]() {
							  ServoStrategy::Write(planner_smoothen_.Plan(smoothen_t_));
							  smoothen_t_ += smoothen_dt_;
							  if(smoothen_t_ > planner_smoothen_.GetDuration()) {
								  smoothen_timer_.stop();
							  }
						  })
			.start();
	}

private:
	PlannerAdapter planner_pusher_{};
	PlannerAdapter planner_retreat_{};
	float interval_duration_sec_{};
	float push_time_{};

	float smoothen_t_ = 0;
	float smoothen_dt_ = 0.02;
	PlannerAdapter planner_smoothen_;
	Timer smoothen_timer_;
};
#endif
