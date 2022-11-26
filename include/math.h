
#ifndef INCLUDE_MATH_H_
#define INCLUDE_MATH_H_
#include <algorithm>

namespace math
{
float map(float x, float in_min, float in_max, float out_min, float out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

} // namespace math
#endif
