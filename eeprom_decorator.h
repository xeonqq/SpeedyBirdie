#include "tuple_helper.h"
#include <ESP_EEPROM.h>
#include <tuple>
#include <utility>

struct ShootingPower {
  ShootingPower(uint16_t power) : shooting_power{power} {}
  ShootingPower() = default;
  uint16_t shooting_power;
};

struct ShootingIntervalSec {
  ShootingIntervalSec(uint8_t shoot_interval_sec)
      : shooting_interval_sec{shoot_interval_sec} {}
  ShootingIntervalSec() = default;
  uint8_t shooting_interval_sec;
};

using EEPROMData = std::tuple<ShootingPower, ShootingIntervalSec>;

namespace detail {
template <size_t I = 0, typename... Ts>
typename std::enable_if<I == sizeof...(Ts), void>::type
ReadValueAtOffset(std::tuple<Ts...> tuple, uint16_t offset) {
  return;
}

template <size_t I = 0, typename... Ts>
typename std::enable_if<(I < sizeof...(Ts)), void>::type
ReadValueAtOffset(std::tuple<Ts...> tuple, uint16_t offset) {
  EEPROM.get(offset, std::get<I>(tuple));
  ReadValueAtOffset<I + 1>(tuple, offset + sizeof(std::get<I>(tuple)));
}

template <typename Tuple, typename T> uint16_t GetOffset() {
  const auto type_index = Index<T, Tuple>::value;
  return SumSizeofComponentsImpl<Tuple>(std::make_index_sequence<type_index>{});
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
    EEPROM.put(detail::GetOffset<EEPROMData, T>(), value);
    EEPROM.commit();
  }
};
