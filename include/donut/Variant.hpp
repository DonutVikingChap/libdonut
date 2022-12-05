#ifndef DONUT_VARIANT_HPP
#define DONUT_VARIANT_HPP

#include <algorithm>        // std::max
#include <cassert>          // assert
#include <compare>          // std::strong_ordering, std::common_comparison_category_t, std::compare_three_way_result_t
#include <cstddef>          // std::size_t, std::byte
#include <cstdint>          // std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t
#include <exception>        // std::exception
#include <functional>       // std::hash, std::invoke
#include <initializer_list> // std::initializer_list
#include <memory>           // std::construct_at, std::destroy_at
#include <new>              // std::launder
#include <optional>         // std::optional
#include <type_traits>      // std::is_..._v, std::false_type, std::true_type, std::integral_constant, std::remove_..._t, std::common_type_t, std::contitional_t
#include <utility>          // std::move, std::forward, std::swap, std::in_place_type_t, std::in_place_index_t, std::index_sequence, std::make_index_sequence

namespace donut {

template <typename... Ts>
class Variant;

namespace detail {

template <template <typename...> typename Template, typename... TemplateArgs>
constexpr void derivedFromTemplateSpecializationTest(const Template<TemplateArgs...>&);

template <typename T, template <typename...> typename Template>
concept derived_from_template_specialization_of = requires(const T& t) {
	derivedFromTemplateSpecializationTest<Template>(t);
};

template <typename T, std::size_t index, typename... Ts>
struct VariantIndexImpl;

template <typename T, std::size_t index, typename First, typename... Rest>
struct VariantIndexImpl<T, index, First, Rest...> : VariantIndexImpl<T, index + 1, Rest...> {};

template <typename T, std::size_t index, typename... Rest>
struct VariantIndexImpl<T, index, T, Rest...> : std::integral_constant<std::size_t, index> {};

template <typename... Functors>
struct Overloaded : Functors... {
	using Functors::operator()...;
};

template <typename... Functors>
Overloaded(Functors...) -> Overloaded<Functors...>;

template <typename V>
struct Matcher {
	V variant;

	template <typename... Functors>
	constexpr decltype(auto) operator()(Functors&&... functors) const {
		return visit(Overloaded{std::forward<Functors>(functors)...}, variant);
	}
};

} // namespace detail

template <typename T, typename V>
struct variant_has_alternative;

template <typename T, typename First, typename... Rest>
struct variant_has_alternative<T, Variant<First, Rest...>> : variant_has_alternative<T, Variant<Rest...>> {};

template <typename T>
struct variant_has_alternative<T, Variant<>> : std::false_type {};

template <typename T, typename... Rest>
struct variant_has_alternative<T, Variant<T, Rest...>> : std::true_type {};

template <typename T, typename V>
inline constexpr bool variant_has_alternative_v = variant_has_alternative<T, V>::value;

template <typename T, typename V>
struct variant_index;

template <typename T, typename... Ts>
struct variant_index<T, Variant<Ts...>> : detail::VariantIndexImpl<T, 0, Ts...> {};

template <typename T, typename V>
inline constexpr std::size_t variant_index_v = variant_index<T, V>::value;

template <std::size_t index, typename V>
struct variant_alternative;

template <std::size_t index, typename First, typename... Rest>
struct variant_alternative<index, Variant<First, Rest...>> : variant_alternative<index - 1, Variant<Rest...>> {};

template <typename T, typename... Rest>
struct variant_alternative<0, Variant<T, Rest...>> {
	using type = T;
};

template <std::size_t index, typename V>
using variant_alternative_t = typename variant_alternative<index, V>::type;

template <typename V>
struct variant_size;

template <typename... Ts>
struct variant_size<Variant<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template <typename V>
inline constexpr std::size_t variant_size_v = variant_size<V>::value;

struct Monostate {};

constexpr bool operator==(Monostate, Monostate) noexcept {
	return true;
}

constexpr std::strong_ordering operator<=>(Monostate, Monostate) noexcept {
	return std::strong_ordering::equal;
}

} // namespace donut

template <>
class std::hash<donut::Monostate> {
public:
	[[nodiscard]] std::size_t operator()(const donut::Monostate&) const {
		return 0;
	}
};

namespace donut {

class BadVariantAccess : public std::exception {
public:
	BadVariantAccess() noexcept = default;

	[[nodiscard]] const char* what() const noexcept override {
		return "Bad variant access.";
	}
};

template <typename... Ts>
class Variant {
private:
	using FirstAlternative = variant_alternative_t<0, Variant>;

	static constexpr bool HAS_DEFAULT_CONSTRUCTOR = std::is_default_constructible_v<FirstAlternative>;
	static constexpr bool HAS_COPY_CONSTRUCTOR = (std::is_copy_constructible_v<Ts> && ...);
	static constexpr bool HAS_MOVE_CONSTRUCTOR = (std::is_move_constructible_v<Ts> && ...);
	static constexpr bool HAS_COPY_ASSIGNMENT = ((std::is_copy_constructible_v<Ts> && std::is_copy_assignable_v<Ts>)&&...);
	static constexpr bool HAS_MOVE_ASSIGNMENT = ((std::is_move_constructible_v<Ts> && std::is_move_assignable_v<Ts>)&&...);
	static constexpr bool HAS_TRIVIAL_COPY_CONSTRUCTOR = (std::is_trivially_copy_constructible_v<Ts> && ...);
	static constexpr bool HAS_TRIVIAL_MOVE_CONSTRUCTOR = (std::is_trivially_move_constructible_v<Ts> && ...);
	static constexpr bool HAS_TRIVIAL_COPY_ASSIGNMENT = ((
		std::is_trivially_copy_constructible_v<Ts> && std::is_trivially_copy_assignable_v<Ts> && std::is_trivially_destructible_v<Ts>)&&...);
	static constexpr bool HAS_TRIVIAL_MOVE_ASSIGNMENT = ((
		std::is_trivially_move_constructible_v<Ts> && std::is_trivially_move_assignable_v<Ts> && std::is_trivially_destructible_v<Ts>)&&...);
	static constexpr bool HAS_TRIVIAL_DESTRUCTOR = (std::is_trivially_destructible_v<Ts> && ...);

	template <typename T>
	struct SelectAlternativeTypeOverload {
		template <typename U>
		T operator()(T, U&& u) requires(requires { T{std::forward<U>(u)}; });
	};

	struct SelectAlternativeTypeOverloadSet : SelectAlternativeTypeOverload<Ts>... {
		using SelectAlternativeTypeOverload<Ts>::operator()...;
	};

	template <typename U>
	static constexpr auto F(U&& u) -> decltype(SelectAlternativeTypeOverloadSet{}(std::forward<U>(u), std::forward<U>(u)));

public:
	// clang-format off
    using index_type =
        std::conditional_t<sizeof...(Ts) < 255ull, std::uint8_t,
        std::conditional_t<sizeof...(Ts) < 65535ull, std::uint16_t,
        std::conditional_t<sizeof...(Ts) < 4294967295ull, std::uint32_t,
        std::uint64_t>>>;
	// clang-format on

	static constexpr index_type npos = -1;

	Variant() noexcept(std::is_nothrow_default_constructible_v<FirstAlternative>) requires(HAS_DEFAULT_CONSTRUCTOR)
		: activeTypeIndex(0) {
		std::construct_at(reinterpret_cast<FirstAlternative*>(&storage));
	}

	template <typename U>
	Variant(U&& value) noexcept(std::is_nothrow_constructible_v<decltype(F(std::forward<U>(value)))>) requires(!std::is_same_v<std::remove_cvref_t<U>, Variant> &&
		variant_has_alternative_v<decltype(F(std::forward<U>(value))), Variant> && std::is_constructible_v<decltype(F(std::forward<U>(value))), U>)
		: activeTypeIndex(variant_index_v<decltype(F(std::forward<U>(value))), Variant>) {
		std::construct_at(reinterpret_cast<decltype(F(std::forward<U>(value)))*>(&storage), std::forward<U>(value));
	}

	template <typename T, typename... Args>
	explicit Variant(std::in_place_type_t<T>, Args&&... args) requires(variant_has_alternative_v<T, Variant>&& std::is_constructible_v<T, Args...>)
		: activeTypeIndex(variant_index_v<T, Variant>) {
		std::construct_at(reinterpret_cast<T*>(&storage), std::forward<Args>(args)...);
	}

	template <typename T, typename U, typename... Args>
	explicit Variant(std::in_place_type_t<T>, std::initializer_list<U> ilist, Args&&... args) requires(
		variant_has_alternative_v<T, Variant>&& std::is_constructible_v<T, std::initializer_list<U>&, Args...>)
		: activeTypeIndex(variant_index_v<T, Variant>) {
		std::construct_at(reinterpret_cast<T*>(&storage), ilist, std::forward<Args>(args)...);
	}

	template <std::size_t index, typename... Args>
	explicit Variant(std::in_place_index_t<index>, Args&&... args) requires(index < sizeof...(Ts) && std::is_constructible_v<variant_alternative_t<index, Variant>, Args...>)
		: activeTypeIndex(index) {
		std::construct_at(reinterpret_cast<variant_alternative_t<index, Variant>*>(&storage), std::forward<Args>(args)...);
	}

	template <std::size_t index, typename U, typename... Args>
	explicit Variant(std::in_place_index_t<index>, std::initializer_list<U> ilist, Args&&... args) requires(
		index < sizeof...(Ts) && std::is_constructible_v<variant_alternative_t<index, Variant>, std::initializer_list<U>&, Args...>)
		: activeTypeIndex(index) {
		std::construct_at(reinterpret_cast<variant_alternative_t<index, Variant>*>(&storage), ilist, std::forward<Args>(args)...);
	}

	~Variant() {
		destroy();
	}

	~Variant() requires(HAS_TRIVIAL_DESTRUCTOR) = default;

	Variant(const Variant& other) requires(!HAS_COPY_CONSTRUCTOR) = delete;
	Variant(const Variant& other) requires(HAS_COPY_CONSTRUCTOR&& HAS_TRIVIAL_COPY_CONSTRUCTOR) = default;
	Variant(const Variant& other) requires(HAS_COPY_CONSTRUCTOR && !HAS_TRIVIAL_COPY_CONSTRUCTOR)
		: activeTypeIndex(other.activeTypeIndex) {
		const auto visitor = [&]<typename T>(const T& value) -> void {
			std::construct_at(reinterpret_cast<T*>(&storage), value);
		};
		[&]<std::size_t... indices>(std::index_sequence<indices...>)->void {
			(void)(((other.template is<indices>()) ? (visitor(other.template as<indices>()), true) : false) || ...);
		}
		(std::make_index_sequence<sizeof...(Ts)>{});
	}

	Variant(Variant&& other) noexcept requires(HAS_MOVE_CONSTRUCTOR&& HAS_TRIVIAL_MOVE_CONSTRUCTOR) = default;
	Variant(Variant&& other) noexcept((std::is_nothrow_move_constructible_v<Ts> && ...)) requires(HAS_MOVE_CONSTRUCTOR && !HAS_TRIVIAL_MOVE_CONSTRUCTOR)
		: activeTypeIndex(other.activeTypeIndex) {
		const auto visitor = [&]<typename T>(T& value) -> void {
			std::construct_at(reinterpret_cast<T*>(&storage), std::move(value));
		};
		[&]<std::size_t... indices>(std::index_sequence<indices...>)->void {
			(void)(((other.template is<indices>()) ? (visitor(other.template as<indices>()), true) : false) || ...);
		}
		(std::make_index_sequence<sizeof...(Ts)>{});
	}

	Variant& operator=(const Variant& other) requires(!HAS_COPY_ASSIGNMENT) = delete;
	Variant& operator=(const Variant& other) requires(HAS_COPY_ASSIGNMENT&& HAS_TRIVIAL_COPY_ASSIGNMENT) = default;
	Variant& operator=(const Variant& other) requires(HAS_COPY_ASSIGNMENT && !HAS_TRIVIAL_COPY_ASSIGNMENT) {
		*this = Variant{other};
		return *this;
	}

	Variant& operator=(Variant&& other) noexcept requires(HAS_MOVE_ASSIGNMENT&& HAS_TRIVIAL_MOVE_ASSIGNMENT) = default;
	Variant& operator=(Variant&& other) noexcept(((std::is_nothrow_move_constructible_v<Ts> && std::is_nothrow_move_assignable_v<Ts>)&&...)) requires(
		HAS_MOVE_ASSIGNMENT && !HAS_TRIVIAL_MOVE_ASSIGNMENT) {
		if (activeTypeIndex == other.activeTypeIndex) {
			const auto visitor = [&]<typename T>(T& value) -> void {
				*std::launder(reinterpret_cast<T*>(&storage)) = std::move(value);
			};
			[&]<std::size_t... Indices>(std::index_sequence<Indices...>)->void {
				(void)(((other.template is<Indices>()) ? (visitor(other.template as<Indices>()), true) : false) || ...);
			}
			(std::make_index_sequence<sizeof...(Ts)>{});
		} else if (other.activeTypeIndex == npos) {
			destroy();
			activeTypeIndex = npos;
		} else {
			destroy();
			if constexpr (((std::is_nothrow_move_constructible_v<Ts> && std::is_nothrow_move_assignable_v<Ts>)&&...)) {
				const auto visitor = [&]<typename T>(T& value) -> void {
					std::construct_at(reinterpret_cast<T*>(&storage), std::move(value));
				};
				[&]<std::size_t... Indices>(std::index_sequence<Indices...>)->void {
					(void)(((other.template is<Indices>()) ? (visitor(other.template as<Indices>()), true) : false) || ...);
				}
				(std::make_index_sequence<sizeof...(Ts)>{});
			} else {
				try {
					const auto visitor = [&]<typename T>(T& value) -> void {
						std::construct_at(reinterpret_cast<T*>(&storage), std::move(value));
					};
					[&]<std::size_t... Indices>(std::index_sequence<Indices...>)->void {
						(void)(((other.template is<Indices>()) ? (visitor(other.template as<Indices>()), true) : false) || ...);
					}
					(std::make_index_sequence<sizeof...(Ts)>{});
				} catch (...) {
					activeTypeIndex = npos;
					throw;
				}
			}
			activeTypeIndex = other.activeTypeIndex;
		}
		return *this;
	}

	template <typename U>
	Variant& operator=(U&& value) noexcept(std::is_nothrow_assignable_v<decltype(F(std::forward<U>(value)))&, U>&&
			std::is_nothrow_constructible_v<decltype(F(std::forward<U>(value))), U>) requires(!std::is_same_v<std::remove_cvref_t<U>, Variant> &&
		variant_has_alternative_v<decltype(F(std::forward<U>(value))), Variant> && std::is_assignable_v<decltype(F(std::forward<U>(value))), U>) {
		using T = decltype(F(std::forward<U>(value)));
		if (is<T>()) {
			as<T>() = std::forward<U>(value);
		} else {
			if constexpr (std::is_nothrow_constructible_v<T, U> || !std::is_nothrow_move_constructible_v<T>) {
				emplace<T>(std::forward<U>(value));
			} else {
				emplace<T>(T(std::forward<U>(value)));
			}
		}
		return *this;
	}

	template <typename T, typename... Args>
	T& emplace(Args&&... args) requires(variant_has_alternative_v<T, Variant>&& std::is_constructible_v<T, Args...>) {
		return emplace<variant_index_v<T, Variant>>(std::forward<Args>(args)...);
	}

	template <typename T, typename U, typename... Args>
	T& emplace(std::initializer_list<U> ilist, Args&&... args) requires(variant_has_alternative_v<T, Variant>&& std::is_constructible_v<T, std::initializer_list<U>&, Args...>) {
		return emplace<variant_index_v<T, Variant>>(ilist, std::forward<Args>(args)...);
	}

	template <std::size_t index, typename... Args>
	variant_alternative_t<index, Variant>& emplace(Args&&... args) requires(std::is_constructible_v<variant_alternative_t<index, Variant>, Args...>) {
		static_assert(index < sizeof...(Ts));
		using T = variant_alternative_t<index, Variant>;
		destroy();
		try {
			std::construct_at(reinterpret_cast<T*>(&storage), std::forward<Args>(args)...);
			activeTypeIndex = index;
		} catch (...) {
			activeTypeIndex = npos;
			throw;
		}
		return *std::launder(reinterpret_cast<T*>(&storage));
	}

	template <std::size_t index, typename U, typename... Args>
	variant_alternative_t<index, Variant>& emplace(std::initializer_list<U> ilist, Args&&... args) requires(
		std::is_constructible_v<variant_alternative_t<index, Variant>, std::initializer_list<U>&, Args...>) {
		static_assert(index < sizeof...(Ts));
		using T = variant_alternative_t<index, Variant>;
		destroy();
		try {
			std::construct_at(reinterpret_cast<T*>(&storage), ilist, std::forward<Args>(args)...);
			activeTypeIndex = index;
		} catch (...) {
			activeTypeIndex = npos;
			throw;
		}
		return *std::launder(reinterpret_cast<T*>(&storage));
	}

	void swap(Variant& other) noexcept(((std::is_nothrow_move_constructible_v<Ts> && std::is_nothrow_swappable_v<Ts>)&&...)) {
		if (activeTypeIndex == other.activeTypeIndex) {
			const auto visitor = [&]<typename T>(T& value) -> void {
				using std::swap;
				swap(*std::launder(reinterpret_cast<T*>(&storage)), value);
			};
			[&]<std::size_t... Indices>(std::index_sequence<Indices...>)->void {
				(void)(((other.template is<Indices>()) ? (visitor(other.template as<Indices>()), true) : false) || ...);
			}
			(std::make_index_sequence<sizeof...(Ts)>{});
		} else {
			Variant old = std::move(*this);
			*this = std::move(other);
			other = std::move(old);
		}
	}

	friend void swap(Variant& a, Variant& b) noexcept(noexcept(a.swap(b))) {
		a.swap(b);
	}

	[[nodiscard]] constexpr index_type index() const noexcept {
		return activeTypeIndex;
	}

	[[nodiscard]] constexpr bool valueless_by_exception() const noexcept {
		return activeTypeIndex == npos;
	}

	template <typename T>
	[[nodiscard]] constexpr bool is() const noexcept requires(variant_has_alternative_v<T, Variant>) {
		return activeTypeIndex == variant_index_v<T, Variant>;
	}

	template <std::size_t index>
	[[nodiscard]] constexpr bool is() const noexcept {
		return activeTypeIndex == index;
	}

	template <typename T>
	[[nodiscard]] T& as() & noexcept requires(variant_has_alternative_v<T, Variant>) {
		assert(is<T>());
		return *std::launder(reinterpret_cast<T*>(&storage));
	}

	template <typename T>
	[[nodiscard]] const T& as() const& noexcept requires(variant_has_alternative_v<T, Variant>) {
		assert(is<T>());
		return *std::launder(reinterpret_cast<const T*>(&storage));
	}

	template <typename T>
	[[nodiscard]] T&& as() && noexcept requires(variant_has_alternative_v<T, Variant>) {
		assert(is<T>());
		return std::move(*std::launder(reinterpret_cast<T*>(&storage)));
	}

	template <typename T>
	[[nodiscard]] const T&& as() const&& noexcept requires(variant_has_alternative_v<T, Variant>) {
		assert(is<T>());
		return std::move(*std::launder(reinterpret_cast<const T*>(&storage)));
	}

	template <std::size_t index>
	[[nodiscard]] variant_alternative_t<index, Variant>& as() & noexcept {
		assert(is<index>());
		return *std::launder(reinterpret_cast<variant_alternative_t<index, Variant>*>(&storage));
	}

	template <std::size_t index>
	[[nodiscard]] const variant_alternative_t<index, Variant>& as() const& noexcept {
		assert(is<index>());
		return *std::launder(reinterpret_cast<const variant_alternative_t<index, Variant>*>(&storage));
	}

	template <std::size_t index>
	[[nodiscard]] variant_alternative_t<index, Variant>&& as() && noexcept {
		assert(is<index>());
		return std::move(*std::launder(reinterpret_cast<variant_alternative_t<index, Variant>*>(&storage)));
	}

	template <std::size_t index>
	[[nodiscard]] const variant_alternative_t<index, Variant>&& as() const&& noexcept {
		assert(is<index>());
		return std::move(*std::launder(reinterpret_cast<const variant_alternative_t<index, Variant>*>(&storage)));
	}

	template <typename T>
	[[nodiscard]] T& get() requires(variant_has_alternative_v<T, Variant>) {
		if (!is<T>()) {
			throw BadVariantAccess{};
		}
		return as<T>();
	}

	template <typename T>
	[[nodiscard]] const T& get() const requires(variant_has_alternative_v<T, Variant>) {
		if (!is<T>()) {
			throw BadVariantAccess{};
		}
		return as<T>();
	}

	template <std::size_t index>
	[[nodiscard]] variant_alternative_t<index, Variant>& get() {
		if (!is<index>()) {
			throw BadVariantAccess{};
		}
		return as<index>();
	}

	template <std::size_t index>
	[[nodiscard]] const variant_alternative_t<index, Variant>& get() const {
		if (!is<index>()) {
			throw BadVariantAccess{};
		}
		return as<index>();
	}

	template <typename T>
	[[nodiscard]] T* get_if() noexcept requires(variant_has_alternative_v<T, Variant>) {
		return (is<T>()) ? &as<T>() : nullptr;
	}

	template <typename T>
	[[nodiscard]] const T* get_if() const noexcept requires(variant_has_alternative_v<T, Variant>) {
		return (is<T>()) ? &as<T>() : nullptr;
	}

	template <std::size_t index>
	[[nodiscard]] variant_alternative_t<index, Variant>* get_if() noexcept {
		return (is<index>()) ? &as<index>() : nullptr;
	}

	template <std::size_t index>
	[[nodiscard]] const variant_alternative_t<index, Variant>* get_if() const noexcept {
		return (is<index>()) ? &as<index>() : nullptr;
	}

	template <typename Visitor, typename V>
	static constexpr decltype(auto) visit(Visitor&& visitor, V&& variant) {
		constexpr bool is_all_lvalue_reference = (std::is_lvalue_reference_v<decltype(std::invoke(std::forward<Visitor>(visitor), std::forward<V>(variant).template as<Ts>()))> &&
			...);
		constexpr bool is_any_const =
			(std::is_const_v<std::remove_reference_t<decltype(std::invoke(std::forward<Visitor>(visitor), std::forward<V>(variant).template as<Ts>()))>> || ...);
		using R = std::common_type_t<decltype(std::invoke(std::forward<Visitor>(visitor), std::forward<V>(variant).template as<Ts>()))...>;
		if constexpr (std::is_void_v<R>) {
			[&]<std::size_t... Indices>(std::index_sequence<Indices...>)->void {
				if (!(((variant.template is<Indices>()) ? (std::invoke(std::forward<Visitor>(visitor), std::forward<V>(variant).template as<Indices>()), true) : false) || ...)) {
					throw BadVariantAccess{};
				}
			}
			(std::make_index_sequence<sizeof...(Ts)>{});
		} else if constexpr (is_all_lvalue_reference) {
			std::conditional_t<is_any_const, const R*, R*> result = nullptr;
			[&]<std::size_t... Indices>(std::index_sequence<Indices...>)->void {
				if (!(((variant.template is<Indices>()) ? ((result = &std::invoke(std::forward<Visitor>(visitor), std::forward<V>(variant).template as<Indices>())), true) :
														  false) ||
						...)) {
					throw BadVariantAccess{};
				}
			}
			(std::make_index_sequence<sizeof...(Ts)>{});
			return *result;
		} else {
			std::optional<R> result{};
			[&]<std::size_t... Indices>(std::index_sequence<Indices...>)->void {
				if (!(((variant.template is<Indices>()) ? (result.emplace(std::invoke(std::forward<Visitor>(visitor), std::forward<V>(variant).template as<Indices>())), true) :
														  false) ||
						...)) {
					throw BadVariantAccess{};
				}
			}
			(std::make_index_sequence<sizeof...(Ts)>{});
			return R{std::move(*result)};
		}
	}

private:
	void destroy() noexcept {
		if constexpr (!HAS_TRIVIAL_DESTRUCTOR) {
			const auto visitor = [&](auto& value) -> void {
				std::destroy_at(&value);
			};
			[&]<std::size_t... Indices>(std::index_sequence<Indices...>)->void {
				(void)(((is<Indices>()) ? (visitor(as<Indices>()), true) : false) || ...);
			}
			(std::make_index_sequence<sizeof...(Ts)>{});
		}
	}

	alignas(std::max({alignof(Ts)...})) std::byte storage[std::max({sizeof(Ts)...})];
	index_type activeTypeIndex;
};

template <typename Visitor, detail::derived_from_template_specialization_of<Variant> V>
constexpr decltype(auto) visit(Visitor&& visitor, V&& variant) {
	return std::remove_cvref_t<V>::visit(std::forward<Visitor>(visitor), std::forward<V>(variant));
}

template <typename T, typename... Ts>
[[nodiscard]] constexpr bool holds_alternative(const Variant<Ts...>& variant) noexcept {
	return variant.template is<T>();
}

template <std::size_t index, typename... Ts>
[[nodiscard]] constexpr bool holds_alternative(const Variant<Ts...>& variant) noexcept {
	return variant.template is<index>();
}

template <typename T, typename... Ts>
[[nodiscard]] constexpr T& get(Variant<Ts...>& variant) {
	return variant.template get<T>();
}

template <typename T, typename... Ts>
[[nodiscard]] constexpr const T& get(const Variant<Ts...>& variant) {
	return variant.template get<T>();
}

template <std::size_t index, typename... Ts>
[[nodiscard]] constexpr variant_alternative_t<index, Variant<Ts...>>* get(Variant<Ts...>& variant) {
	return variant.template get<index>();
}

template <std::size_t index, typename... Ts>
[[nodiscard]] constexpr const variant_alternative_t<index, Variant<Ts...>>* get(const Variant<Ts...>& variant) {
	return variant.template get<index>();
}

template <typename T, typename... Ts>
[[nodiscard]] constexpr T* get_if(Variant<Ts...>* variant) noexcept {
	assert(variant);
	return variant->template get_if<T>();
}

template <typename T, typename... Ts>
[[nodiscard]] constexpr const T* get_if(const Variant<Ts...>* variant) noexcept {
	assert(variant);
	return variant->template get_if<T>();
}

template <std::size_t index, typename... Ts>
[[nodiscard]] constexpr variant_alternative_t<index, Variant<Ts...>>* get_if(Variant<Ts...>* variant) noexcept {
	assert(variant);
	return variant->template get_if<index>();
}

template <std::size_t index, typename... Ts>
[[nodiscard]] constexpr const variant_alternative_t<index, Variant<Ts...>>* get_if(const Variant<Ts...>* variant) noexcept {
	assert(variant);
	return variant->template get_if<index>();
}

template <typename... Ts>
[[nodiscard]] constexpr bool operator==(const Variant<Ts...>& a, const Variant<Ts...>& b) {
	if (a.index() != b.index()) {
		return false;
	}
	if (a.valueless_by_exception()) {
		return true;
	}
	return visit([&]<typename T>(const T& value) -> bool { return value == b.template as<T>(); }, a);
}

template <typename... Ts>
[[nodiscard]] constexpr std::common_comparison_category_t<std::compare_three_way_result_t<Ts>...> operator<=>(const Variant<Ts...>& a, const Variant<Ts...>& b) {
	if (a.valueless_by_exception() && b.valueless_by_exception()) {
		return std::strong_ordering::equal;
	}
	if (a.valueless_by_exception()) {
		return std::strong_ordering::less;
	}
	if (b.valueless_by_exception()) {
		return std::strong_ordering::greater;
	}
	if (a.index() != b.index()) {
		return a.index() <=> b.index();
	}
	return visit([&]<typename T>(const T& value) -> std::common_comparison_category_t<std::compare_three_way_result_t<Ts>...> { return value <=> b.template as<T>(); }, a);
}

template <typename V>
[[nodiscard]] constexpr detail::Matcher<V> match(V&& variant) {
	return detail::Matcher<V>{std::forward<V>(variant)};
}

} // namespace donut

template <typename... Ts>
class std::hash<donut::Variant<Ts...>> {
public:
	[[nodiscard]] std::size_t operator()(const donut::Variant<Ts...>& variant) const {
		return visit(hasher, variant);
	}

private:
	template <typename T>
	class Hash {
	public:
		[[nodiscard]] std::size_t operator()(const T& value) const {
			return hasher(value);
		}

	private:
		[[no_unique_address]] std::hash<T> hasher{};
	};

	class HashVisitor : public Hash<Ts>... {
	public:
		using Hash<Ts>::operator()...;
	};

	[[no_unique_address]] HashVisitor hasher{};
};

#endif
