#ifndef INCLUDE_EEPROM_DECORATOR_H_
#define INCLUDE_EEPROM_DECORATOR_H_
#include <tuple_helper.h>
#include <tuple>
#include <utility>

#define DEBUG_EEPROM

template <typename T> struct EEPROMBaseType {
	EEPROMBaseType(const T& v) : value{v}
	{
	}
	EEPROMBaseType() = default;
	using type = T;
	operator type() const
	{
		return value;
	}
	T value;
};

struct ShootingPower : EEPROMBaseType<uint16_t> {
	using EEPROMBaseType<uint16_t>::EEPROMBaseType;
	static constexpr char const* name = "shooting_power";
};

struct ShootingIntervalSec : EEPROMBaseType<float> {
	using EEPROMBaseType<float>::EEPROMBaseType;
	static constexpr char const* name = "shooting_interval_sec";
};

struct LeftMotorOffset : EEPROMBaseType<uint16_t> {
	using EEPROMBaseType<uint16_t>::EEPROMBaseType;
	static constexpr char const* name = "left_motor_offset";
};

struct RightMotorOffset : EEPROMBaseType<uint16_t> {
	using EEPROMBaseType<uint16_t>::EEPROMBaseType;
	static constexpr char const* name = "right_motor_offset";
};

struct ServoEndPosition : EEPROMBaseType<float> {
	using EEPROMBaseType<float>::EEPROMBaseType;
	static constexpr char const* name = "servo_end_position";
};

struct BallReleaseServoStartPosition : EEPROMBaseType<float> {
	using EEPROMBaseType<float>::EEPROMBaseType;
	static constexpr char const* name = "ball_release_servo_start_position";
};

struct BallReleaseToPushTimeDelay : EEPROMBaseType<float> {
	using EEPROMBaseType<float>::EEPROMBaseType;
	static constexpr char const* name = "ball_release_to_push_time_delay";
};

using EEPROMData = std::tuple<ShootingPower, ShootingIntervalSec, LeftMotorOffset, RightMotorOffset, ServoEndPosition,
							  BallReleaseServoStartPosition, BallReleaseToPushTimeDelay>;

#endif
