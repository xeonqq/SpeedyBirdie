#include "tuple_helper.h"
#include <ArduinoJson.h>
#include <ESP_EEPROM.h>
#include <tuple>
#include <utility>

#define DEBUG_EEPROM

template <typename T> struct EEPROMBaseType {
  EEPROMBaseType(const T &v) : value{v} {}
  EEPROMBaseType() = default;
  using type = T;
  operator type() const { return value; }
  T value;
};

struct ShootingPower : EEPROMBaseType<uint16_t> {
  using EEPROMBaseType<uint16_t>::EEPROMBaseType;
  static constexpr char const *name = "shooting_power";
};

struct ShootingIntervalSec : EEPROMBaseType<float> {
  using EEPROMBaseType<float>::EEPROMBaseType;
  static constexpr char const *name = "shooting_interval_sec";
};

struct LeftMotorOffset : EEPROMBaseType<uint16_t> {
  using EEPROMBaseType<uint16_t>::EEPROMBaseType;
  static constexpr char const *name = "left_motor_offset";
};

struct RightMotorOffset : EEPROMBaseType<uint16_t> {
  using EEPROMBaseType<uint16_t>::EEPROMBaseType;
  static constexpr char const *name = "right_motor_offset";
};

struct ServoEndPosition : EEPROMBaseType<float> {
  using EEPROMBaseType<float>::EEPROMBaseType;
  static constexpr char const *name = "servo_end_position";
};

struct BallReleaseServoStartPosition : EEPROMBaseType<float> {
  using EEPROMBaseType<float>::EEPROMBaseType;
  static constexpr char const *name = "ball_release_servo_start_position";
};

struct BallReleaseToPushTimeDelay : EEPROMBaseType<float> {
  using EEPROMBaseType<float>::EEPROMBaseType;
  static constexpr char const *name = "ball_release_to_push_time_delay";
};

using EEPROMData =
    std::tuple<ShootingPower, ShootingIntervalSec, LeftMotorOffset,
               RightMotorOffset, ServoEndPosition,
               BallReleaseServoStartPosition, BallReleaseToPushTimeDelay>;

namespace detail {
template <size_t I = 0, typename... Ts>
typename std::enable_if<I == sizeof...(Ts), void>::type
ReadAllDataAfterOffset(std::tuple<Ts...> &tuple, uint16_t offset) {
  return;
}

template <size_t I = 0, typename... Ts>
typename std::enable_if<(I < sizeof...(Ts)), void>::type
ReadAllDataAfterOffset(std::tuple<Ts...> &tuple, uint16_t offset) {
  EEPROM.get(offset, std::get<I>(tuple));
  constexpr auto size = sizeof(std::tuple_element_t<I, std::tuple<Ts...>>);
#ifdef DEBUG_EEPROM
  Serial.print("read offset:");
  Serial.print(offset);
  Serial.print(" with value:");
  Serial.print(std::get<I>(tuple).value);
  Serial.print(" with size:");
  Serial.println(size);
#endif
  ReadAllDataAfterOffset<I + 1>(tuple, offset + size);
}

template <typename Tuple, typename T>
typename std::enable_if<(Index<T, Tuple>::value > 0), uint16_t>::type
GetOffset() {
  const auto type_index = Index<T, Tuple>::value;
  return SumSizeofComponentsImpl<Tuple>(std::make_index_sequence<type_index>{});
};

template <typename Tuple, typename T>
typename std::enable_if<(Index<T, Tuple>::value == 0), uint16_t>::type
GetOffset() {
  return 0;
};

} // namespace detail

String ToJsonBuf(const EEPROMData &eeprom_data) {
  DynamicJsonDocument json(std::tuple_size_v<EEPROMData> * 30);
  for_each_in_tuple(
      eeprom_data, [&json](const auto &data) { json[data.name] = data.value; });

  String buf;
  serializeJson(json, buf);
  return buf;
}

class EEPROMDecorator {
public:
  EEPROMDecorator() {}
  void Init() { EEPROM.begin(SumSizeofComponents<EEPROMData>()); }

  EEPROMData ReadData() {
    EEPROMData data;
    detail::ReadAllDataAfterOffset(data, 0);
    return data;
  }

  template <typename T> T Read() {
    T value{};
    const auto offset = detail::GetOffset<EEPROMData, T>();
    EEPROM.get(offset, value);
    return value;
  }

  template <typename Struct, typename T> void Write(const T &value) {
    const auto offset = detail::GetOffset<EEPROMData, Struct>();
    EEPROM.put(offset, static_cast<typename Struct::type>(value));
#ifdef DEBUG_EEPROM
    Serial.print("write offset:");
    Serial.print(offset);
    Serial.print(" with value:");
    Serial.println(value);
#endif
    const auto ok = EEPROM.commit();
    if (!ok) {
      Serial.println("EEPROM commit failed");
    }
  }
};
