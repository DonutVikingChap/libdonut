#ifndef DONUT_UNICODE_HPP
#define DONUT_UNICODE_HPP

#include <array>       // std::array
#include <cstddef>     // std::size_t, std::ptrdiff_t
#include <iterator>    // std::iterator_traits, std::input_iterator_tag, std::forward_iterator_tag
#include <optional>    // std::optional
#include <string_view> // std::string_view, std::u8string_view
#include <type_traits> // std::is_same_v
#include <utility>     // std::pair

namespace donut {
namespace unicode {

[[nodiscard]] constexpr bool isValidCodePoint(char32_t codePoint) noexcept {
	return codePoint <= 0x10FFFF && (codePoint < 0xD800 || codePoint > 0xDFFF);
}

inline constexpr char32_t CODE_POINT_ERROR{0xFFFFFFFF};

template <typename InputIt, typename Sentinel>
[[nodiscard]] constexpr std::pair<char32_t, InputIt> readCodePointFromUTF8(InputIt it, Sentinel end) {
	if (it == end) {
		[[unlikely]] return {CODE_POINT_ERROR, it}; // Reached end.
	}
	char32_t codePoint{};
	const char8_t c0 = static_cast<char8_t>(*it++);
	if ((c0 & 0b10000000u) == 0) { // 0-127
		[[likely]] codePoint = c0;
	} else if ((c0 & 0b11100000u) == 0b11000000u) { // 128-2047
		if (it == end) {
			[[unlikely]] return {CODE_POINT_ERROR, it}; // Missing continuation.
		}
		const char8_t c1 = static_cast<char8_t>(*it++);
		if ((c1 & 0b11000000u) != 0b10000000u) {
			[[unlikely]] return {CODE_POINT_ERROR, it}; // Invalid continuation.
		}
		codePoint = ((c0 & 0b00011111u) << 6) | (c1 & 0b00111111u);
		if (codePoint < 128) {
			[[unlikely]] return {CODE_POINT_ERROR, it}; // Overlong sequence.
		}
	} else if ((c0 & 0b11110000u) == 0b11100000u) { // 2048-65535
		if (it == end) {
			[[unlikely]] return {CODE_POINT_ERROR, it}; // Missing continuation.
		}
		const char8_t c1 = static_cast<char8_t>(*it++);
		if (it == end) {
			[[unlikely]] return {CODE_POINT_ERROR, it}; // Missing continuation.
		}
		const char8_t c2 = static_cast<char8_t>(*it++);
		if ((c1 & 0b11000000u) != 0b10000000u || (c2 & 0b11000000u) != 0b10000000u) {
			[[unlikely]] return {CODE_POINT_ERROR, it}; // Invalid continuation.
		}
		codePoint = ((c0 & 0b00001111u) << 12) | ((c1 & 0b00111111u) << 6) | (c2 & 0b00111111u);
		if (codePoint < 2048) {
			[[unlikely]] return {CODE_POINT_ERROR, it}; // Overlong sequence.
		}
		if (codePoint >= 0xD800 && codePoint <= 0xDFFF) {
			[[unlikely]] return {CODE_POINT_ERROR, it}; // Surrogate code point.
		}
	} else if ((c0 & 0b11111000u) == 0b11110000u) { // 65536-1114111
		if (it == end) {
			[[unlikely]] return {CODE_POINT_ERROR, it}; // Missing continuation.
		}
		const char8_t c1 = static_cast<char8_t>(*it++);
		if (it == end) {
			[[unlikely]] return {CODE_POINT_ERROR, it}; // Missing continuation.
		}
		const char8_t c2 = static_cast<char8_t>(*it++);
		if (it == end) {
			[[unlikely]] return {CODE_POINT_ERROR, it}; // Missing continuation.
		}
		const char8_t c3 = static_cast<char8_t>(*it++);
		if ((c1 & 0b11000000u) != 0b10000000u || (c2 & 0b11000000u) != 0b10000000u || (c3 & 0b11000000u) != 0b10000000u) {
			[[unlikely]] return {CODE_POINT_ERROR, it}; // Invalid continuation.
		}
		codePoint = ((c0 & 0b00000111u) << 18) | ((c1 & 0b00111111u) << 12) | ((c2 & 0b00111111u) << 6) | (c3 & 0b00111111u);
		if (codePoint < 65536) {
			[[unlikely]] return {CODE_POINT_ERROR, it}; // Overlong sequence.
		}
		if (codePoint > 1114111) {
			[[unlikely]] return {CODE_POINT_ERROR, it}; // Invalid code point.
		}
	} else {
		[[unlikely]] return {CODE_POINT_ERROR, it}; // Invalid code unit.
	}
	return {codePoint, it};
}

struct UTF8FromCodePointResult {
	std::array<char8_t, 4> codeUnits;
	std::size_t size;
};

[[nodiscard]] constexpr UTF8FromCodePointResult getUTF8FromCodePoint(char32_t codePoint) noexcept {
	if (codePoint <= 0x7F) {
		[[likely]] return {
			.codeUnits{
				static_cast<char8_t>(codePoint),
			},
			.size = 1,
		};
	}
	if (codePoint <= 0x7FF) {
		return {
			.codeUnits{
				static_cast<char8_t>((codePoint >> 6) + 192),
				static_cast<char8_t>((codePoint & 63) + 128),
			},
			.size = 2,
		};
	}
	if (codePoint <= 0xFFFF) {
		return {
			.codeUnits{
				static_cast<char8_t>((codePoint >> 12) + 224),
				static_cast<char8_t>(((codePoint >> 6) & 63) + 128),
				static_cast<char8_t>((codePoint & 63) + 128),
			},
			.size = 3,
		};
	}
	return {
		.codeUnits{
			static_cast<char8_t>((codePoint >> 18) + 240),
			static_cast<char8_t>(((codePoint >> 12) & 63) + 128),
			static_cast<char8_t>(((codePoint >> 6) & 63) + 128),
			static_cast<char8_t>((codePoint & 63) + 128),
		},
		.size = 4,
	};
}

struct UTF8Sentinel {};

template <typename It, typename Sentinel = It>
class UTF8Iterator {
public:
	using difference_type = std::ptrdiff_t;
	using value_type = char32_t;
	using reference = const value_type&;
	using pointer = const value_type*;
	using iterator_category = std::forward_iterator_tag;
	using sentinel = UTF8Sentinel;

	constexpr UTF8Iterator() = default;

	constexpr UTF8Iterator(It it, Sentinel end)
		: it(it)
		, next(it)
		, end(end) {
		++*this;
	}

	[[nodiscard]] constexpr bool operator==(const UTF8Iterator& other) const {
		return it == other.it;
	}

	[[nodiscard]] constexpr bool operator==(const UTF8Sentinel&) const {
		return it == end;
	}

	[[nodiscard]] constexpr reference operator*() const {
		return codePoint;
	}

	[[nodiscard]] constexpr pointer operator->() const {
		return &**this;
	}

	constexpr UTF8Iterator& operator++() {
		it = next;
		const auto [newCodePoint, newNext] = readCodePointFromUTF8(next, end);
		codePoint = newCodePoint;
		next = newNext;
		return *this;
	}

	constexpr UTF8Iterator operator++(int) {
		UTF8Iterator old = *this;
		++*this;
		return old;
	}

	[[nodiscard]] constexpr It base() const {
		return it;
	}

private:
	It it{};
	It next{};
	Sentinel end{};
	char32_t codePoint{};
};

template <typename It, typename Sentinel>
requires std::is_same_v<typename std::iterator_traits<It>::iterator_category, std::input_iterator_tag>
class UTF8Iterator<It, Sentinel> {
public:
	using difference_type = std::ptrdiff_t;
	using value_type = char32_t;
	using reference = const value_type&;
	using pointer = const value_type*;
	using iterator_category = std::input_iterator_tag;
	using sentinel = UTF8Sentinel;

	constexpr UTF8Iterator() = default;

	constexpr UTF8Iterator(It it, Sentinel end)
		: it(it)
		, end(end) {}

	[[nodiscard]] constexpr bool operator==(const UTF8Iterator& other) const {
		if (it == end || other.it == other.end) {
			return it == other.it && static_cast<bool>(codePoint) == static_cast<bool>(other.codePoint);
		}
		ensureCodePoint();
		other.ensureCodePoint();
		return it == other.it;
	}

	[[nodiscard]] constexpr bool operator==(const UTF8Sentinel&) const {
		return it == end && !codePoint;
	}

	[[nodiscard]] constexpr reference operator*() const {
		ensureCodePoint();
		return *codePoint;
	}

	[[nodiscard]] constexpr pointer operator->() const {
		return &**this;
	}

	constexpr UTF8Iterator& operator++() {
		if (!codePoint) {
			const auto [newCodePoint, newIt] = readCodePointFromUTF8(it, end);
			it = newIt;
		}
		codePoint.reset();
		return *this;
	}

	constexpr UTF8Iterator operator++(int) {
		ensureCodePoint();
		UTF8Iterator old = *this;
		codePoint.reset();
		return old;
	}

private:
	void ensureCodePoint() const {
		if (!codePoint) {
			const auto [newCodePoint, newIt] = readCodePointFromUTF8(it, end);
			codePoint = newCodePoint;
			it = newIt;
		}
	}

	mutable It it{};
	Sentinel end{};
	mutable std::optional<char32_t> codePoint{};
};

class UTF8View {
public:
	using iterator = UTF8Iterator<const char8_t*>;
	using difference_type = typename iterator::difference_type;
	using value_type = typename iterator::value_type;
	using reference = typename iterator::reference;
	using pointer = typename iterator::pointer;
	using iterator_category = typename iterator::iterator_category;
	using sentinel = typename iterator::sentinel;

	constexpr UTF8View() noexcept = default;

	constexpr explicit UTF8View(std::u8string_view str) noexcept
		: it(str.data(), str.data() + str.size()) {}

	explicit UTF8View(std::string_view str) noexcept
		: it(reinterpret_cast<const char8_t*>(str.data()), reinterpret_cast<const char8_t*>(str.data() + str.size())) {
		static_assert(sizeof(char) == sizeof(char8_t));
		static_assert(alignof(char) == alignof(char8_t));
	}

	[[nodiscard]] constexpr const iterator& begin() const noexcept {
		return it;
	}

	[[nodiscard]] constexpr sentinel end() const noexcept { // NOLINT(readability-convert-member-functions-to-static)
		return {};
	}

private:
	UTF8Iterator<const char8_t*> it{};
};

} // namespace unicode
} // namespace donut

#endif
