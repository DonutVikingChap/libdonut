#ifndef DONUT_UNICODE_HPP
#define DONUT_UNICODE_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <string_view>

namespace donut {
namespace unicode {

inline constexpr char32_t CODE_POINT_ERROR{0xFFFFFFFF};

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

	constexpr UTF8Iterator() noexcept = default;

	constexpr UTF8Iterator(It it, Sentinel end) noexcept
		: it(it)
		, next(it)
		, end(end) {
		++*this;
	}

	[[nodiscard]] constexpr bool operator==(const UTF8Iterator& other) const noexcept {
		return it == other.it;
	}

	[[nodiscard]] constexpr bool operator==(const UTF8Sentinel&) const noexcept {
		return it == end;
	}

	[[nodiscard]] constexpr reference operator*() const noexcept {
		return codePoint;
	}

	[[nodiscard]] constexpr pointer operator->() const noexcept {
		return &**this;
	}

	constexpr UTF8Iterator& operator++() noexcept {
		it = next;
		codePoint = nextCodePoint();
		return *this;
	}

	constexpr UTF8Iterator operator++(int) noexcept {
		UTF8Iterator old = *this;
		++*this;
		return old;
	}

	[[nodiscard]] constexpr It base() const noexcept {
		return it;
	}

private:
	constexpr char32_t nextCodePoint() noexcept {
		if (next == end) {
			[[unlikely]] return CODE_POINT_ERROR; // Reached end.
		}
		char32_t result{};
		const char8_t c0 = *next++;
		if ((c0 & 0b10000000) == 0) { // 0-127
			[[likely]] result = c0;
		} else if ((c0 & 0b11100000) == 0b11000000) { // 128-2047
			if (next == end) {
				[[unlikely]] return CODE_POINT_ERROR; // Missing continuation.
			}
			const char8_t c1 = *next++;
			if ((c1 & 0b11000000) != 0b10000000) {
				[[unlikely]] return CODE_POINT_ERROR; // Invalid continuation.
			}
			result = ((c0 & 0b11111)) << 6 | (c1 & 0b111111);
			if (result < 128) {
				[[unlikely]] return CODE_POINT_ERROR; // Overlong sequence.
			}
		} else if ((c0 & 0b11110000) == 0b11100000) { // 2048-65535
			if (next == end) {
				[[unlikely]] return CODE_POINT_ERROR; // Missing continuation.
			}
			const char8_t c1 = *next++;
			if (next == end) {
				[[unlikely]] return CODE_POINT_ERROR; // Missing continuation.
			}
			const char8_t c2 = *next++;
			if ((c1 & 0b11000000) != 0b10000000 || (c2 & 0b11000000) != 0b10000000) {
				[[unlikely]] return CODE_POINT_ERROR; // Invalid continuation.
			}
			result = ((c0 & 0b1111) << 12) | ((c1 & 0b111111) << 6) | (c2 & 0b111111);
			if (result < 2048) {
				[[unlikely]] return CODE_POINT_ERROR; // Overlong sequence.
			}
			if (result >= 0xD800 && result <= 0xDFFF) {
				[[unlikely]] return CODE_POINT_ERROR; // Surrogate code point.
			}
		} else if ((c0 & 0b11111000) == 0b11110000) { // 65536-1114111
			if (next == end) {
				[[unlikely]] return CODE_POINT_ERROR; // Missing continuation.
			}
			const char8_t c1 = *next++;
			if (next == end) {
				[[unlikely]] return CODE_POINT_ERROR; // Missing continuation.
			}
			const char8_t c2 = *next++;
			if (next == end) {
				[[unlikely]] return CODE_POINT_ERROR; // Missing continuation.
			}
			const char8_t c3 = *next++;
			if ((c1 & 0b11000000) != 0b10000000 || (c2 & 0b11000000) != 0b10000000 || (c3 & 0b11000000) != 0b10000000) {
				[[unlikely]] return CODE_POINT_ERROR; // Invalid continuation.
			}
			result = ((c0 & 0b111) << 18) | ((c1 & 0b111111) << 12) | ((c2 & 0b111111) << 6) | (c3 & 0b111111);
			if (result < 65536) {
				[[unlikely]] return CODE_POINT_ERROR; // Overlong sequence.
			}
			if (result > 1114111) {
				[[unlikely]] return CODE_POINT_ERROR; // Invalid code point.
			}
		} else {
			[[unlikely]] return CODE_POINT_ERROR; // Invalid byte.
		}
		return result;
	}

	It it = nullptr;
	It next = nullptr;
	Sentinel end = nullptr;
	char32_t codePoint = 0;
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

[[nodiscard]] constexpr bool isValidCodePoint(std::uint32_t codePointValue) noexcept {
	return codePointValue <= 0x10FFFF && (codePointValue < 0xD800 || codePointValue > 0xDFFF);
}

struct UTF8FromCodePointResult {
	std::array<char8_t, 4> codeUnits;
	std::size_t size;
};

[[nodiscard]] constexpr UTF8FromCodePointResult getUTF8FromCodePoint(char32_t codePoint) noexcept {
	if (codePoint <= 0x7F) {
		[[likely]];
		return {
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

} // namespace unicode
} // namespace donut

#endif
