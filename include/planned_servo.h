#ifndef INCLUDE_PS_H_
#define INCLUDE_PS_H_
#include "planner_adapter.h"

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
		now = std::fmod(now, GetIntervalDurationSec());
		float new_position;
		const auto pushing_time = GetPushingTime();
		const auto idle_time = GetIdleTime();
		if(now < idle_time) {
			new_position = planner_pusher_.Plan(0);
		} else if(now < (idle_time + pushing_time)) {
			new_position = planner_pusher_.Plan(now - idle_time);
		} else if(now < GetIntervalDurationSec()) {
			new_position = planner_retreat_.Plan(now - idle_time - pushing_time);
		}
		// Serial.println(new_position);

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

	void InitSmoothen()
	{
		//MicroQt::eventLoop.enqueueEvent([this]() { WaitForSmoothen(); });
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

private:
	void WaitForSmoothen()
	{
		/*MicroQt::Synchronizer synchronizer;*/
		/*float start = ServoStrategy::Read();*/
		/*float end = planner_pusher_.GetStart();*/
		/*float duration = 2.F;*/
		/*PlannerAdapter planner_smoothen;*/
		/*planner_smoothen.Init(duration, start, end);*/
		/*float t = 0;*/
		/*float dt = 0.02;*/
		/*MicroQt::Timer timer;*/
		/*auto conn = timer.sglTimeout.connect([&]() {*/
		/*ServoStrategy::Write(planner_smoothen.Plan(t));*/
		/*t += dt;*/
		/*if(t > planner_smoothen.GetDuration()) {*/
		/*synchronizer.exit(1);*/
		/*}*/
		/*});*/
		/*timer.start(dt * 1000);*/
		/*synchronizer.exec();*/
		/*timer.sglTimeout.disconnect(conn);*/
	}

	PlannerAdapter planner_pusher_{};
	PlannerAdapter planner_retreat_{};
	float interval_duration_sec_{};
	float push_time_{};
};
#endif
