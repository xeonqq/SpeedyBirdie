#include <ESP_EEPROM.h>
#include <tuple>

struct ShootingPower {
  uint16_t shooting_power;
};

struct ShootingIntervalSec {
  uint8_t shooting_interval_sec;
};

using EEPROMData = std::tuple<ShootingPower, ShootingIntervalSec>;

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

template <typename T, size_t... Is>
auto SumSizeofComponentsImpl(T const &t, std::index_sequence<Is...>) {
  return (sizeof(std::get<Is>(t)) + ...);
}

template <typename... Args>
int SumSizeofComponents(const std::tuple<Args...> &t) {
  return SumSizeofComponentsImpl(t, std::index_sequence_for<Args...>{});
}

class EEPROMDecorator {

public:
  EEPROMDecorator() {}
  void Init() { EEPROM.begin(SumSizeofComponents(data_)); }

  const EEPROMData &ReadData() { ReadValueAtOffset(data_, 0); }

  template <typename T> void Write(const T &value) {
    // EEPROM.put(, value);
    // EEPROM.commit();
  }

private:
  EEPROMData data_;
};
