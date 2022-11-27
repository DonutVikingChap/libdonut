#ifndef DONUT_REFLECTION_HPP
#define DONUT_REFLECTION_HPP

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace donut {

namespace detail {

struct Init {
	template <typename T>
	operator T();
};

template <std::size_t Index>
struct AggregateSizeTag : AggregateSizeTag<Index - 1> {};

template <>
struct AggregateSizeTag<0> {};

// clang-format off

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<26>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) {
	return 26;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<25>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) {
	return 25;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<24>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) {
	return 24;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<23>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) {
	return 23;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<22>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) {
	return 22;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<21>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) {
	return 21;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<20>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) {
	return 20;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<19>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) {
	return 19;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<18>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) {
	return 18;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<17>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) {
	return 17;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<16>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) {
	return 16;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<15>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) {
	return 15;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<14>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) {
	return 14;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<13>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) {
	return 13;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<12>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) {
	return 12;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<11>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) {
	return 11;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<10>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) {
	return 10;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<9>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) {
	return 9;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<8>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) {
	return 8;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<7>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) {
	return 7;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<6>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) {
	return 6;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<5>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) {
	return 5;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<4>) -> decltype(T{Init{}, Init{}, Init{}, Init{}}, std::size_t{}) {
	return 4;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<3>) -> decltype(T{Init{}, Init{}, Init{}}, std::size_t{}) {
	return 3;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<2>) -> decltype(T{Init{}, Init{}}, std::size_t{}) {
	return 2;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<1>) -> decltype(T{Init{}}, std::size_t{}) {
	return 1;
}

template <typename T>
constexpr auto aggregateSizeImpl(AggregateSizeTag<0>) -> decltype(T{}, std::size_t{}) {
	return 0;
}

// clang-format on

template <typename T>
constexpr std::size_t aggregateSize() {
	static_assert(std::is_aggregate_v<T>);
	return aggregateSizeImpl<T>(detail::AggregateSizeTag<26>{});
}

} // namespace detail

template <typename T>
inline constexpr std::size_t aggregate_size_v = detail::aggregateSize<T>();

[[nodiscard]] constexpr auto fields(auto&& aggregate) {
	using T = std::remove_cvref_t<decltype(aggregate)>;
	static_assert(std::is_aggregate_v<T>);
	if constexpr (aggregate_size_v<T> == 26) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
	} else if constexpr (aggregate_size_v<T> == 25) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y);
	} else if constexpr (aggregate_size_v<T> == 24) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x);
	} else if constexpr (aggregate_size_v<T> == 23) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w);
	} else if constexpr (aggregate_size_v<T> == 22) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v);
	} else if constexpr (aggregate_size_v<T> == 21) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u);
	} else if constexpr (aggregate_size_v<T> == 20) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t);
	} else if constexpr (aggregate_size_v<T> == 19) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s);
	} else if constexpr (aggregate_size_v<T> == 18) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r);
	} else if constexpr (aggregate_size_v<T> == 17) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q);
	} else if constexpr (aggregate_size_v<T> == 16) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);
	} else if constexpr (aggregate_size_v<T> == 15) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
	} else if constexpr (aggregate_size_v<T> == 14) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n);
	} else if constexpr (aggregate_size_v<T> == 13) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m);
	} else if constexpr (aggregate_size_v<T> == 12) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l);
	} else if constexpr (aggregate_size_v<T> == 11) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k);
	} else if constexpr (aggregate_size_v<T> == 10) {
		auto&& [a, b, c, d, e, f, g, h, i, j] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j);
	} else if constexpr (aggregate_size_v<T> == 9) {
		auto&& [a, b, c, d, e, f, g, h, i] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i);
	} else if constexpr (aggregate_size_v<T> == 8) {
		auto&& [a, b, c, d, e, f, g, h] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h);
	} else if constexpr (aggregate_size_v<T> == 7) {
		auto&& [a, b, c, d, e, f, g] = aggregate;
		return std::tie(a, b, c, d, e, f, g);
	} else if constexpr (aggregate_size_v<T> == 6) {
		auto&& [a, b, c, d, e, f] = aggregate;
		return std::tie(a, b, c, d, e, f);
	} else if constexpr (aggregate_size_v<T> == 5) {
		auto&& [a, b, c, d, e] = aggregate;
		return std::tie(a, b, c, d, e);
	} else if constexpr (aggregate_size_v<T> == 4) {
		auto&& [a, b, c, d] = aggregate;
		return std::tie(a, b, c, d);
	} else if constexpr (aggregate_size_v<T> == 3) {
		auto&& [a, b, c] = aggregate;
		return std::tie(a, b, c);
	} else if constexpr (aggregate_size_v<T> == 2) {
		auto&& [a, b] = aggregate;
		return std::tie(a, b);
	} else if constexpr (aggregate_size_v<T> == 1) {
		auto&& [a] = aggregate;
		return std::tie(a);
	} else {
		return std::tie();
	}
}

constexpr void tupleForEach(auto&& tuple, auto fn) {
	[&]<std::size_t... Indices>(std::index_sequence<Indices...>) {
		(fn(std::get<Indices>(tuple)), ...);
	}
	(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<decltype(tuple)>>>{});
}

[[nodiscard]] constexpr auto tupleTransform(auto&& tuple, auto fn) {
	return [&]<std::size_t... Indices>(std::index_sequence<Indices...>) {
		return std::forward_as_tuple(fn(std::get<Indices>(tuple))...);
	}
	(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<decltype(tuple)>>>{});
}

template <std::size_t N>
constexpr void forEachIndex(auto fn) {
	[&]<std::size_t... Indices>(std::index_sequence<Indices...>) {
		(fn(std::integral_constant<std::size_t, Indices>{}), ...);
	}
	(std::make_index_sequence<N>{});
}

} // namespace donut

#endif
