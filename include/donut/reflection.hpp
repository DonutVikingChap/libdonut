#ifndef DONUT_REFLECTION_HPP
#define DONUT_REFLECTION_HPP

#include <cstddef>     // std::size_t
#include <tuple>       // std::tuple_size_v, std::make_tuple, std::tie, std::get(std::tuple)
#include <type_traits> // std::is_aggregate_v, std::integral_constant, std::remove_cvref_t
#include <utility>     // std::index_sequence, std::make_index_sequence

namespace donut {
namespace reflection {

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
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag<26>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) { return 26; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag<25>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) { return 25; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag<24>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) { return 24; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag<23>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) { return 23; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag<22>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) { return 22; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag<21>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) { return 21; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag<20>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) { return 20; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag<19>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) { return 19; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag<18>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) { return 18; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag<17>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) { return 17; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag<16>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) { return 16; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag<15>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) { return 15; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag<14>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) { return 14; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag<13>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) { return 13; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag<12>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) { return 12; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag<11>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) { return 11; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag<10>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) { return 10; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag< 9>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) { return 9; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag< 8>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) { return 8; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag< 7>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) { return 7; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag< 6>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) { return 6; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag< 5>) -> decltype(T{Init{}, Init{}, Init{}, Init{}, Init{}}, std::size_t{}) { return 5; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag< 4>) -> decltype(T{Init{}, Init{}, Init{}, Init{}}, std::size_t{}) { return 4; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag< 3>) -> decltype(T{Init{}, Init{}, Init{}}, std::size_t{}) { return 3; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag< 2>) -> decltype(T{Init{}, Init{}}, std::size_t{}) { return 2; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag< 1>) -> decltype(T{Init{}}, std::size_t{}) { return 1; }
template <typename T> constexpr auto aggregateSizeImpl(AggregateSizeTag< 0>) -> decltype(T{}, std::size_t{}) { return 0; }
// clang-format on

} // namespace detail

/// \cond
template <typename T>
requires std::is_aggregate_v<T> //
struct aggregate_size : std::integral_constant<std::size_t, detail::aggregateSizeImpl<T>(detail::AggregateSizeTag<26>{})> {};
/// \endcond

/**
 * The number of fields in a given aggregate type.
 *
 * \tparam T aggregate type to get the number of fields in.
 *
 * \warning Only sizes up to 26 are supported. If the template is instantiated
 *          with an aggregate type containing more than 26 fields, the program
 *          is ill-formed.
 */
template <typename T>
inline constexpr std::size_t aggregate_size_v = aggregate_size<T>::value;

/**
 * Get a tuple of references to each of the fields of an aggregate.
 *
 * \param aggregate a forwarding reference to an object of aggregate type.
 *
 * \return a tuple where each element is an lvalue reference to the respective
 *         field of the aggregate, based on the declaration order of the fields.
 *
 * \warning Only aggregate sizes up to 26 are supported. If the function is
 *          instantiated with an aggregate type containing more than 26 fields,
 *          the program is ill-formed.
 */
[[nodiscard]] constexpr auto fields(auto&& aggregate) noexcept {
	using T = std::remove_cvref_t<decltype(aggregate)>;
	static_assert(std::is_aggregate_v<T>);
	constexpr std::size_t FIELD_COUNT = aggregate_size_v<T>;
	if constexpr (FIELD_COUNT == 26) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
	} else if constexpr (FIELD_COUNT == 25) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y);
	} else if constexpr (FIELD_COUNT == 24) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x);
	} else if constexpr (FIELD_COUNT == 23) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w);
	} else if constexpr (FIELD_COUNT == 22) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v);
	} else if constexpr (FIELD_COUNT == 21) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u);
	} else if constexpr (FIELD_COUNT == 20) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t);
	} else if constexpr (FIELD_COUNT == 19) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s);
	} else if constexpr (FIELD_COUNT == 18) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r);
	} else if constexpr (FIELD_COUNT == 17) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q);
	} else if constexpr (FIELD_COUNT == 16) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);
	} else if constexpr (FIELD_COUNT == 15) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
	} else if constexpr (FIELD_COUNT == 14) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m, n] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n);
	} else if constexpr (FIELD_COUNT == 13) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l, m] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m);
	} else if constexpr (FIELD_COUNT == 12) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k, l] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k, l);
	} else if constexpr (FIELD_COUNT == 11) {
		auto&& [a, b, c, d, e, f, g, h, i, j, k] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j, k);
	} else if constexpr (FIELD_COUNT == 10) {
		auto&& [a, b, c, d, e, f, g, h, i, j] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i, j);
	} else if constexpr (FIELD_COUNT == 9) {
		auto&& [a, b, c, d, e, f, g, h, i] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h, i);
	} else if constexpr (FIELD_COUNT == 8) {
		auto&& [a, b, c, d, e, f, g, h] = aggregate;
		return std::tie(a, b, c, d, e, f, g, h);
	} else if constexpr (FIELD_COUNT == 7) {
		auto&& [a, b, c, d, e, f, g] = aggregate;
		return std::tie(a, b, c, d, e, f, g);
	} else if constexpr (FIELD_COUNT == 6) {
		auto&& [a, b, c, d, e, f] = aggregate;
		return std::tie(a, b, c, d, e, f);
	} else if constexpr (FIELD_COUNT == 5) {
		auto&& [a, b, c, d, e] = aggregate;
		return std::tie(a, b, c, d, e);
	} else if constexpr (FIELD_COUNT == 4) {
		auto&& [a, b, c, d] = aggregate;
		return std::tie(a, b, c, d);
	} else if constexpr (FIELD_COUNT == 3) {
		auto&& [a, b, c] = aggregate;
		return std::tie(a, b, c);
	} else if constexpr (FIELD_COUNT == 2) {
		auto&& [a, b] = aggregate;
		return std::tie(a, b);
	} else if constexpr (FIELD_COUNT == 1) {
		auto&& [a] = aggregate;
		return std::tie(a);
	} else {
		return std::tie();
	}
}

/**
 * Execute a function once for each index in the sequence from 0 up to, but not
 * including, a given count N.
 *
 * The callback function is passed an instance of
 * std::integral_constant<std::size_t, I> where I is the corresponding index.
 *
 * \tparam N the number of indices in the sequence.
 *
 * \param fn the function to execute. Must accept an object of type
 *        std::integral_constant<std::size_t, I> as a parameter, where I is any
 *        integer from 0 up to, but not including, N.
 *
 * \throws any exception thrown by fn.
 */
template <std::size_t N>
constexpr void forEachIndex(auto fn) {
	[&]<std::size_t... Indices>(std::index_sequence<Indices...>)->void {
		(fn(std::integral_constant<std::size_t, Indices>{}), ...);
	}
	(std::make_index_sequence<N>{});
}

/**
 * Execute a function once for each element in a given tuple, sequentially.
 *
 * The callback function is passed a reference to the element at each respective
 * index of the tuple.
 *
 * \param tuple the tuple to iterate.
 * \param fn the function to execute. Must accept a reference to each of the
 *        types in the tuple as a parameter.
 *
 * \throws any exception thrown by fn.
 */
constexpr void forEach(auto&& tuple, auto fn) {
	[&]<std::size_t... Indices>(std::index_sequence<Indices...>) {
		(fn(std::get<Indices>(tuple)), ...);
	}
	(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<decltype(tuple)>>>{});
}

/**
 * Execute a function for each element in a given tuple and return a tuple
 * containing the results.
 *
 * The callback function is passed a reference to the element at each respective
 * index of the tuple.
 *
 * \param tuple the tuple to transform.
 * \param fn the function to execute. Must accept a reference to each of the
 *        types in the tuple as a parameter and return a non-void value.
 *
 * \throws any exception thrown by fn or by making a tuple from the results.
 */
[[nodiscard]] constexpr auto transform(auto&& tuple, auto fn) {
	return [&]<std::size_t... Indices>(std::index_sequence<Indices...>) {
		return std::make_tuple(fn(std::get<Indices>(tuple))...);
	}
	(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<decltype(tuple)>>>{});
}

} // namespace reflection
} // namespace donut

#endif
