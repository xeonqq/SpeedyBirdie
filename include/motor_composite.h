#ifndef INCLUDE_MC_H_
#define INCLUDE_MC_H_
#include <Libraries/Servo/ServoChannel.h>
class Motors
{
public:
	Motors(uint8_t left_motor_pin, uint8_t right_motor_pin, int us_min, int us_max)
		: min_microseconds_{us_min}, max_microseconds_{us_max}
	{
		// initialize stored last value, before attach
		ForEach([us_min, us_max](auto& motor) {
			motor.setMinValue(us_min);
			motor.setMaxValue(us_max);
		});

		motors_[0].attach(left_motor_pin);
		motors_[1].attach(right_motor_pin);
	}

	void Stop()
	{
		ForEach([this](auto& motor) { motor.setValue(min_microseconds_); });
	}

	void Write(int power)
	{
		//Serial << "power:" << power;
		//Serial.println();
		ForEachI([&](uint16_t i, auto& motor) {
			const int speed_microseconds = map(power, 0, 1000, min_microseconds_ + offsets_[i], max_microseconds_);
			//Serial << "motor:" << speed_microseconds;
			//Serial.println();
			motor.setValue(ConstrainPWM(speed_microseconds));
		});
	}

	void WriteRaw(uint16_t motor_index, int power)
	{
		const int speed_microseconds = map(power, 0, 1000, min_microseconds_, max_microseconds_);
		motors_[motor_index].setValue(ConstrainPWM(speed_microseconds));
	}

	void SetPwmOffsets(const std::array<uint16_t, 2>& offsets)
	{
		offsets_ = offsets;
	}

	int Read()
	{
		return motors_[0].getValue() - min_microseconds_;
	}

private:
	int ConstrainPWM(int value)
	{
		return constrain(value, min_microseconds_, 1300);
	}

	template <typename Func> void ForEachI(Func&& func)
	{
		for(auto i = 0; i < motors_.size(); ++i) {
			std::forward<Func>(func)(i, motors_[i]);
		}
	}
	template <typename Func> void ForEach(Func&& func)
	{
		std::for_each(motors_.begin(), motors_.end(), std::forward<Func>(func));
	}

	std::array<ServoChannel, 2> motors_{};
	std::array<uint16_t, 2> offsets_{};

	const int max_microseconds_{2000};
	const int min_microseconds_{1000};
};
#endif
