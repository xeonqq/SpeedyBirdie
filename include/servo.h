#ifndef INCLUDE_SERVO_H_
#define INCLUDE_SERVO_H_

#include "math.h"
#include <Libraries/Servo/ServoChannel.h>

class FeederServo
{
public:
	FeederServo(int pin, int us_min = 1000, int us_max = 2000) : us_min_{us_min}, us_max_{us_max}, pin_{pin}
	{
		servo.attach(pin);
		servo.setMinValue(us_min_);
		servo.setMaxValue(us_max_);
	}

	void Reset()
	{
		WriteMicroseconds(us_min_);
	}

	void Write(float percentage)
	{
		const auto us_input = math::map(percentage, 0.0F, 1.F, us_min_, us_max_);
		// Serial.print("servo write:");
		// Serial.println(us_input);

		unsigned long currentTime = micros();
		WriteMicroseconds(us_input);

		//Serial.print(F("WriteMicroseconds"));
		//Serial.print((micros() - currentTime) / 1000.0);
		//Serial.println(F(" ms"));
	}
	static float GetNetualPositionPercentage()
	{
		return 0.5;
	}

	void WriteMicroseconds(uint16_t us)
	{
		// Serial.print("servo write:");
		// Serial.println(us);
		servo.setValue(ConstrainPWM(us));
	}

	float Read()
	{
		return math::map(static_cast<float>(servo.getValue()), static_cast<float>(us_min_), static_cast<float>(us_max_),
						 0.F, 1.F);
	}

private:
	int ConstrainPWM(int value)
	{
		return constrain(value, us_min_, us_max_);
	}
	int us_min_;
	int us_max_;
	int pin_;
	ServoChannel servo;
};
#endif
