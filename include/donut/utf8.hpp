#ifndef DONUT_UTF8_HPP
#define DONUT_UTF8_HPP

#include <cstddef>
#include <iterator>
#include <memory>
#include <string_view>

namespace donut {

inline constexpr char32_t UTF8_ERROR{0xFFFFFFFF};

class UTF8Iterator {
public:
	using difference_type = std::ptrdiff_t;
	using value_type = char32_t;
	using reference = const value_type&;
	using pointer = const value_type*;
	using iterator_category = std::forward_iterator_tag;
	struct sentinel {};

	constexpr UTF8Iterator() noexcept = default;

	constexpr UTF8Iterator(const char8_t* it, const char8_t* end) noexcept
		: it(it)
		, next(it)
		, end(end) {
		++*this;
	}

	[[nodiscard]] constexpr bool operator==(const UTF8Iterator& other) const noexcept {
		return it == other.it;
	}

	[[nodiscard]] constexpr bool operator==(const sentinel&) const noexcept {
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

private:
	constexpr char32_t nextCodePoint() noexcept {
		if (next == end) {
			[[unlikely]] return UTF8_ERROR; // Reached end.
		}
		char32_t result{};
		const char8_t c0 = *next++;
		if ((c0 & 0b10000000) == 0) { // 0-127
			[[likely]] result = c0;
		} else if ((c0 & 0b11100000) == 0b11000000) { // 128-2047
			if (end - next < 1) {
				next = end;
				[[unlikely]] return UTF8_ERROR; // Missing continuation.
			}
			const char8_t c1 = *next++;
			if ((c1 & 0b11000000) != 0b10000000) {
				[[unlikely]] return UTF8_ERROR; // Invalid continuation.
			}
			result = ((c0 & 0b11111)) << 6 | (c1 & 0b111111);
			if (result < 128) {
				[[unlikely]] return UTF8_ERROR; // Overlong sequence.
			}
		} else if ((c0 & 0b11110000) == 0b11100000) { // 2048-65535
			if (end - next < 2) {
				next = end;
				[[unlikely]] return UTF8_ERROR; // Missing continuation.
			}
			const char8_t c1 = *next++;
			const char8_t c2 = *next++;
			if ((c1 & 0b11000000) != 0b10000000 || (c2 & 0b11000000) != 0b10000000) {
				[[unlikely]] return UTF8_ERROR; // Invalid continuation.
			}
			result = ((c0 & 0b1111) << 12) | ((c1 & 0b111111) << 6) | (c2 & 0b111111);
			if (result < 2048) {
				[[unlikely]] return UTF8_ERROR; // Overlong sequence.
			}
			if (result >= 0xD800 && result <= 0xDFFF) {
				[[unlikely]] return UTF8_ERROR; // Surrogate code point.
			}
		} else if ((c0 & 0b11111000) == 0b11110000) { // 65536-1114111
			if (end - next < 3) {
				next = end;
				[[unlikely]] return UTF8_ERROR; // Missing continuation.
			}
			const char8_t c1 = *next++;
			const char8_t c2 = *next++;
			const char8_t c3 = *next++;
			if ((c1 & 0b11000000) != 0b10000000 || (c2 & 0b11000000) != 0b10000000 || (c3 & 0b11000000) != 0b10000000) {
				[[unlikely]] return UTF8_ERROR; // Invalid continuation.
			}
			result = ((c0 & 0b111) << 18) | ((c1 & 0b111111) << 12) | ((c2 & 0b111111) << 6) | (c3 & 0b111111);
			if (result < 65536) {
				[[unlikely]] return UTF8_ERROR; // Overlong sequence.
			}
			if (result > 1114111) {
				[[unlikely]] return UTF8_ERROR; // Invalid code point.
			}
		} else {
			[[unlikely]] return UTF8_ERROR; // Invalid byte.
		}
		return result;
	}

	const char8_t* it = nullptr;
	const char8_t* next = nullptr;
	const char8_t* end = nullptr;
	char32_t codePoint = 0;
};

class UTF8View {
public:
	using iterator = UTF8Iterator;
	using difference_type = iterator::difference_type;
	using value_type = iterator::value_type;
	using reference = iterator::reference;
	using pointer = iterator::pointer;
	using iterator_category = iterator::iterator_category;
	using sentinel = iterator::sentinel;

	constexpr UTF8View() noexcept = default;

	constexpr UTF8View(std::u8string_view str) noexcept
		: it(str.data(), str.data() + str.size()) {}

	explicit UTF8View(std::string_view str) noexcept
		: it(reinterpret_cast<const char8_t*>(str.data()), reinterpret_cast<const char8_t*>(str.data() + str.size())) {}

	[[nodiscard]] constexpr const iterator& begin() const noexcept {
		return it;
	}

	[[nodiscard]] constexpr sentinel end() const noexcept { // NOLINT(readability-convert-member-functions-to-static)
		return {};
	}

private:
	UTF8Iterator it{};
};

} // namespace donut

#endif
