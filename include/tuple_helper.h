#ifndef INCLUDE_TUPLE_HELPER_H_
#define INCLUDE_TUPLE_HELPER_H_
#include <tuple>
#include <utility>
template <class T, class Tuple> struct Index;

template <class T, class... Types> struct Index<T, std::tuple<T, Types...>> {
	static const std::size_t value = 0;
};

template <class T, class U, class... Types> struct Index<T, std::tuple<U, Types...>> {
	static const std::size_t value = 1 + Index<T, std::tuple<Types...>>::value;
};

uint16_t Sum()
{
	return 0;
}

template <typename T, typename... Us> uint16_t Sum(T&& t, Us&&... us)
{
	return std::forward<T>(t) + Sum(std::forward<Us>(us)...);
}

template <typename Tuple, size_t... Is> auto SumSizeofComponentsImpl(std::index_sequence<Is...>)
{
	return Sum(sizeof(std::tuple_element_t<Is, Tuple>)...);
}
/*template <typename Tuple, size_t... Is>*/
/*auto SumSizeofComponentsImpl(std::index_sequence<Is...>) {*/
/*return (sizeof(std::tuple_element_t<Is, Tuple>) + ...);*/
/*}*/

template <typename Tuple> int SumSizeofComponents()
{
	constexpr auto size = std::tuple_size<Tuple>{};
	return SumSizeofComponentsImpl<Tuple>(std::make_index_sequence<size>{});
}

namespace detail
{
template <int... Is> struct seq {
};

template <int N, int... Is> struct gen_seq : gen_seq<N - 1, N - 1, Is...> {
};

template <int... Is> struct gen_seq<0, Is...> : seq<Is...> {
};

template <typename T, typename F, int... Is> void for_each(T&& t, F f, seq<Is...>)
{
	auto l = {(f(std::get<Is>(t)), 0)...};
}
} // namespace detail

template <typename... Ts, typename F> void for_each_in_tuple(std::tuple<Ts...>& t, F f)
{
	detail::for_each(t, f, detail::gen_seq<sizeof...(Ts)>());
}
#endif
