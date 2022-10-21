#include <tuple>
#include <utility>
template <class T, class Tuple> struct Index;

template <class T, class... Types> struct Index<T, std::tuple<T, Types...>> {
  static const std::size_t value = 0;
};

template <class T, class U, class... Types>
struct Index<T, std::tuple<U, Types...>> {
  static const std::size_t value = 1 + Index<T, std::tuple<Types...>>::value;
};

uint16_t Sum() { return 0; }

template <typename T, typename... Us> uint16_t Sum(T &&t, Us &&... us) {
  return std::forward<T>(t) + Sum(std::forward<Us>(us)...);
}

template <typename Tuple, size_t... Is>
auto SumSizeofComponentsImpl(std::index_sequence<Is...>) {
  return Sum(sizeof(std::tuple_element_t<Is, Tuple>)...);
}
/*template <typename Tuple, size_t... Is>*/
/*auto SumSizeofComponentsImpl(std::index_sequence<Is...>) {*/
/*return (sizeof(std::tuple_element_t<Is, Tuple>) + ...);*/
/*}*/

template <typename Tuple> int SumSizeofComponents() {
  constexpr auto size = std::tuple_size<Tuple>{};
  return SumSizeofComponentsImpl<Tuple>(std::make_index_sequence<size>{});
}
