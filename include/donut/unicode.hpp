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

/**
 * Check if a 32-bit unsigned integer value falls within the valid ranges for a
 * Unicode code point.
 *
 * \param codePoint 32-bit code point value to check.
 *
 * \return true if the code unit is a valid code point, false otherwise.
 */
[[nodiscard]] constexpr bool isValidCodePoint(char32_t codePoint) noexcept {
	return codePoint <= 0x10FFFF && (codePoint < 0xD800 || codePoint > 0xDFFF);
}

/**
 * Invalid code point value, used as a return value in Unicode decoding
 * algorithms for conveying encoding errors.
 */
inline constexpr char32_t CODE_POINT_ERROR{0xFFFFFFFF};

/**
 * Decode a single Unicode code point from an iterator of UTF-8 code units in a
 * UTF-8-encoded string.
 *
 * \param it input iterator to a sequence of UTF-8 code units. The expression
 *        `*it++` must be convertible to char8_t.
 * \param end end iterator or sentinel that marks the end of the UTF-8 code unit
 *        sequence.
 *
 * \return a pair where:
 *         - the first element contains the decoded Unicode code point, or
 *           #CODE_POINT_ERROR on failure to decode a code point due to an
 *           encoding error in the UTF-8 string, and
 *         - the second element contains the input iterator, positioned at the
 *           start of the next UTF-8 code unit after the parsed code point
 *           sequence.
 *
 * \throws any exception thrown by the iterator implementation.
 */
template <typename InputIt, typename Sentinel>
[[nodiscard]] constexpr std::pair<char32_t, InputIt> decodeCodePointFromUTF8(InputIt it, Sentinel end) {
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

/**
 * Result of the encodeUTF8FromCodePoint() function.
 */
struct EncodeUTF8FromCodePointResult {
	std::array<char8_t, 4> codeUnits; ///< Array of UTF-8 code units that encode the given code point.
	std::size_t size;                 ///< The length of the encoded code unit sequence stored in the codeUnits array.
};

/**
 * Encode a Unicode code point into a sequence of UTF-8 code units.
 *
 * \param codePoint code point to encode.
 *
 * \return a struct containing an array of up to 4 UTF-8 code units along with a
 *         size that defines the length of the sequence in the array, starting
 *         at index 0.
 *
 * \note The returned array of code units is NOT guaranteed to be
 *       null-terminated. The size value must be used to determine the actual
 *       length of the code point sequence.
 */
[[nodiscard]] constexpr EncodeUTF8FromCodePointResult encodeUTF8FromCodePoint(char32_t codePoint) noexcept {
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

/**
 * Sentinel type for UTF8Iterator.
 */
struct UTF8Sentinel {};

/**
 * Iterator type for decoding Unicode code points from a UTF-8 string, wrapping
 * an existing iterator for UTF-8 code units.
 *
 * \tparam It underlying UTF-8 code unit iterator type to wrap.
 * \tparam Sentinel sentinel type for It.
 */
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
		const auto [newCodePoint, newNext] = decodeCodePointFromUTF8(next, end);
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

/**
 * Specialization of UTF8Iterator that works even for input iterators.
 *
 * \tparam It underlying UTF-8 code unit input iterator type to wrap.
 * \tparam Sentinel sentinel type for It.
 */
template <typename It, typename Sentinel>
requires std::is_same_v<typename std::iterator_traits<It>::iterator_category, std::input_iterator_tag> //
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
			const auto [newCodePoint, newIt] = decodeCodePointFromUTF8(it, end);
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
			const auto [newCodePoint, newIt] = decodeCodePointFromUTF8(it, end);
			codePoint = newCodePoint;
			it = newIt;
		}
	}

	mutable It it{};
	Sentinel end{};
	mutable std::optional<char32_t> codePoint{};
};

/**
 * Non-owning view type for decoding Unicode code points from a contiguous UTF-8
 * string.
 */
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
