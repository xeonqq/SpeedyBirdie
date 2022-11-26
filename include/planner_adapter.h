#ifndef INCLUDE_PA_H_
#define INCLUDE_PA_H_
#include "minimum_jerk_trajectory_planner.h"

class PlannerAdapter : private MinimumJerkTrajortoryPlanner
{
public:
	PlannerAdapter() : MinimumJerkTrajortoryPlanner{}
	{
	}

	void Init(float duration_sec, float start, float end)
	{
		MinimumJerkTrajortoryPlanner::Init(duration_sec, abs((end - start)));
		start_ = start;
		end_ = end;
	}

	void InitByStartAndEnd(float start, float end)
	{
		MinimumJerkTrajortoryPlanner::Init(GetDuration(), abs((end - start)));
		start_ = start;
		end_ = end;
	}

	float Plan(float t) const
	{
		if(start_ < end_) {
			return MinimumJerkTrajortoryPlanner::Plan(t) + start_;
		} else {
			// go reverse when start from big to small for mjt planner
			return MinimumJerkTrajortoryPlanner::Plan(GetDuration() - t) + end_;
		}
	}

	using MinimumJerkTrajortoryPlanner::GetDuration;
	using MinimumJerkTrajortoryPlanner::InitByDuration;

	float GetStart() const
	{
		return start_;
	}

	float GetEnd() const
	{
		return end_;
	}

private:
	float start_;
	float end_;
};
#endif
