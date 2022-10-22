#include "tuple_helper.h"
#include <ArduinoJson.h>
#include <ESP_EEPROM.h>
#include <tuple>
#include <utility>

struct ShootingPower {
  using type = uint16_t;
  ShootingPower(type power) : value{power} {}
  ShootingPower() = default;
  static constexpr char const *name = "shooting_power";
  type value;
};

struct ShootingIntervalSec {
  using type = uint8_t;
  ShootingIntervalSec(type shoot_interval_sec) : value{shoot_interval_sec} {}
  ShootingIntervalSec() = default;
  static constexpr char const *name = "shooting_interval_sec";
  type value;
};

using EEPROMData = std::tuple<ShootingPower, ShootingIntervalSec>;

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
  Serial.print("read offset:");
  Serial.print(offset);
  Serial.print(" with value:");
  Serial.print(std::get<I>(tuple).value);
  constexpr auto size = sizeof(std::tuple_element_t<I, std::tuple<Ts...>>);
  Serial.print(" with size:");
  Serial.println(size);
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

  template <typename T> void Write(const T &value) {

    const auto offset = detail::GetOffset<EEPROMData, T>();
    EEPROM.put(offset, value);
    Serial.print("write offset:");
    Serial.print(offset);
    Serial.print(" with value:");
    Serial.println(value.value);
    const auto ok = EEPROM.commit();
    if (!ok) {
      Serial.println("EEPROM commit failed");
    }
  }
};
