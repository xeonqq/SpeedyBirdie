#include "tuple_helper.h"
#include <ESP_EEPROM.h>
#include <tuple>
#include <utility>

struct ShootingPower {
  ShootingPower(uint16_t power) : value{power} {}
  ShootingPower() = default;
  uint16_t value;
};

struct ShootingIntervalSec {
  ShootingIntervalSec(uint8_t shoot_interval_sec) : value{shoot_interval_sec} {}
  ShootingIntervalSec() = default;
  uint8_t value;
};

using EEPROMData = std::tuple<ShootingPower, ShootingIntervalSec>;

namespace detail {
template <size_t I = 0, typename... Ts>
typename std::enable_if<I == sizeof...(Ts), void>::type
ReadValueAtOffset(std::tuple<Ts...> &tuple, uint16_t offset) {
  return;
}

template <size_t I = 0, typename... Ts>
typename std::enable_if<(I < sizeof...(Ts)), void>::type
ReadValueAtOffset(std::tuple<Ts...> &tuple, uint16_t offset) {
  EEPROM.get(offset, std::get<I>(tuple));
  // Serial.print("read offset:");
  // Serial.print(offset);
  // Serial.print(" with value:");
  // Serial.print(std::get<I>(tuple).value);
  constexpr auto size = sizeof(std::tuple_element_t<I, std::tuple<Ts...>>);
  // Serial.print(" with size:");
  // Serial.println(size);
  ReadValueAtOffset<I + 1>(tuple, offset + size);
}

template <typename Tuple, typename T>
typename std::enable_if<(Index<T, Tuple>::value > 0), uint16_t>::type
GetOffset() {
  const auto type_index = Index<T, Tuple>::value;
  return SumSizeofComponentsImpl<Tuple>(
      std::make_index_sequence<type_index - 1>{});
};

template <typename Tuple, typename T>
typename std::enable_if<(Index<T, Tuple>::value == 0), uint16_t>::type
GetOffset() {
  return 0;
};

} // namespace detail

class EEPROMDecorator {
public:
  EEPROMDecorator() {}
  void Init() { EEPROM.begin(SumSizeofComponents<EEPROMData>()); }

  EEPROMData ReadData() {
    EEPROMData data;
    detail::ReadValueAtOffset(data, 0);
    return data;
  }

  template <typename T> void Write(const T &value) {
    const auto offset = detail::GetOffset<EEPROMData, T>();
    EEPROM.put(offset, value);
    // Serial.print("write offset:");
    // Serial.print(offset);
    // Serial.print(" with value:");
    // Serial.println(value.value);
    const auto ok = EEPROM.commit();
    if (!ok) {
      Serial.println("EEPROM commit failed");
    }
  }
};
