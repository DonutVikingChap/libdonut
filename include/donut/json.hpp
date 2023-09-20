#ifndef DONUT_JSON_HPP
#define DONUT_JSON_HPP

#include <donut/Variant.hpp>
#include <donut/reflection.hpp>
#include <donut/unicode.hpp>

#include <algorithm>        // std::sort, std::equal_range, std::lower_bound, std::upper_bound
#include <array>            // std::array
#include <charconv>         // std::from_chars_result, std::from_chars
#include <cmath>            // std::isnan, std::isinf, std::signbit
#include <compare>          // std::partial_ordering, std::compare_partial_order_fallback
#include <cstddef>          // std::size_t, std::nullptr_t
#include <cstdint>          // std::uint8_t, std::uint32_t
#include <cstdlib>          // std::strtoull, std::strtod
#include <cstring>          // std::memcpy
#include <fmt/format.h>     // fmt::format_to
#include <initializer_list> // std::initializer_list
#include <istream>          // std::istream
#include <iterator>         // std::begin, std::end, std::istreambuf_iterator, std::ostreambuf_iterator
#include <limits>           // std::numeric_limits
#include <numeric>          // std::accumulate
#include <optional>         // std::optional
#include <ostream>          // std::ostream
#include <span>             // std::span, std::as_bytes
#include <sstream>          // std::istringstream, std::ostringstream
#include <stdexcept>        // std::runtime_error, std::out_of_range
#include <string>           // std::...string
#include <string_view>      // std::...string_view
#include <system_error>     // std::errc
#include <tuple>            // std::forward_as_tuple
#include <type_traits>      // std::is_same_v, std::is_arithmetic_v, std::is_pointer_v, std::is_aggregate_v, std::is_constructible_v, std::remove_cvref_t
#include <utility>          // std::pair, std::move, std::forward, std::piecewise_construct
#include <vector>           // std::vector, std::erase(std::vector), std::erase_if(std::vector)

namespace donut::json {

namespace detail {

template <typename T>
concept number = std::is_arithmetic_v<T> && !std::is_same_v<T, bool> && !std::is_same_v<T, char> && !std::is_same_v<T, char8_t> && !std::is_same_v<T, char16_t> &&
                 !std::is_same_v<T, char32_t> && !std::is_same_v<T, wchar_t>;

// clang-format off
struct AlwaysTrue { constexpr bool operator()(const auto&...) const noexcept { return true; } };
struct Get { constexpr const auto& operator()(const auto& value) const noexcept { return value; } };
struct GetFirst { constexpr const auto& operator()(const auto& kv) const noexcept { return kv.first; } };
struct GetSecond { constexpr const auto& operator()(const auto& kv) const noexcept { return kv.second; } };
// clang-format on

} // namespace detail

/**
 * Line and column numbers of a location in a JSON source string.
 */
struct SourceLocation {
	/**
	 * Line number, starting at 1 for the first line. A value of 0 means no
	 * particular line.
	 */
	std::size_t lineNumber;

	/**
	 * Column number, starting at 1 for the first column. A value of 0 means no
	 * particular column.
	 */
	std::size_t columnNumber;

	/**
	 * Compare this source location to another for equality.
	 *
	 * \param other the source location to compare this one to.
	 *
	 * \return true if the source locations are equal, false otherwise.
	 */
	[[nodiscard]] constexpr bool operator==(const SourceLocation& other) const = default;
};

/**
 * Exception type for errors originating from the JSON API.
 */
struct Error : std::runtime_error {
	/**
	 * Location in the JSON source string that the error originated from, or
	 * (0, 0) if the error did not originate from a specific location.
	 */
	SourceLocation source;

	Error(const std::string& message, const SourceLocation& source)
		: std::runtime_error(message)
		, source(source) {}

	Error(const char* message, const SourceLocation& source)
		: std::runtime_error(message)
		, source(source) {}
};

/**
 * Options for JSON serialization.
 */
struct SerializationOptions {
	/**
	 * The starting indentation level, expressed as the number of indentation
	 * characters.
	 */
	std::size_t indentation = 0;

	/**
	 * The number of indentation characters that each new level of indentation
	 * will add.
	 */
	std::size_t relativeIndentation = 4;

	/**
	 * The character to use when performing indentation.
	 */
	char indentationCharacter = ' ';

	/**
	 * Format the output in a way that is nicely human-readable.
	 *
	 * Disable to use a more compact layout without whitespace or indentation.
	 *
	 * \sa prettyPrintMaxSingleLineObjectPropertyCount
	 * \sa prettyPrintMaxSingleLineArrayItemCount
	 */
	bool prettyPrint = true;

	/**
	 * Maximum size of an object before it is split into multiple lines when
	 * pretty printing.
	 *
	 * When set to a positive value, objects at or below this size will be
	 * written in a single line. Set to 0 to always split non-empty objects into
	 * multiple lines regardless of size.
	 *
	 * \note This option only applies when #prettyPrint is true.
	 *
	 * \sa prettyPrint
	 */
	std::size_t prettyPrintMaxSingleLineObjectPropertyCount = 4;

	/**
	 * Maximum size of an array before it is split into multiple lines when
	 * pretty printing.
	 *
	 * When set to a positive value, arrays at or below this size will be
	 * written in a single line. Set to 0 to always split non-empty arrays into
	 * multiple lines regardless of size.
	 *
	 * \note This option only applies when #prettyPrint is true.
	 *
	 * \sa prettyPrint
	 */
	std::size_t prettyPrintMaxSingleLineArrayItemCount = 4;
};

/**
 * Options for JSON deserialization.
 */
struct DeserializationOptions {};

// Forward declaration of the definition below, so that Object and Array can contain objects of type Value through indirection, despite Value being defined in terms of them.
class Value;

/**
 * JSON null type.
 */
using Null = Monostate;

/**
 * JSON boolean type.
 */
using Boolean = bool;

/**
 * JSON string type.
 */
using String = std::string;

/**
 * JSON number type.
 */
using Number = double;

/**
 * JSON object type whose API mimics that of std::multimap<String, Value>.
 */
class Object {
public:
	using key_type = String;
	using mapped_type = Value;
	using value_type = std::pair<String, Value>;
	using size_type = typename std::vector<value_type>::size_type;
	using difference_type = typename std::vector<value_type>::difference_type;
	using reference = typename std::vector<value_type>::reference;
	using const_reference = typename std::vector<value_type>::const_reference;
	using pointer = typename std::vector<value_type>::pointer;
	using const_pointer = typename std::vector<value_type>::const_pointer;
	using iterator = typename std::vector<value_type>::iterator;
	using const_iterator = typename std::vector<value_type>::const_iterator;
	using reverse_iterator = typename std::vector<value_type>::reverse_iterator;
	using const_reverse_iterator = typename std::vector<value_type>::const_reverse_iterator;

	Object() noexcept;
	~Object();

	template <typename InputIt>
	Object(InputIt first, InputIt last);
	Object(std::initializer_list<value_type> ilist);

	Object(const Object& other);
	Object(Object&& other) noexcept;
	Object& operator=(const Object& other);
	Object& operator=(Object&& other) noexcept;

	Object& operator=(std::initializer_list<value_type> ilist);

	[[nodiscard]] Value& at(std::string_view name);
	[[nodiscard]] const Value& at(std::string_view name) const;
	[[nodiscard]] Value& operator[](const String& k);
	[[nodiscard]] Value& operator[](String&& k);

	[[nodiscard]] iterator begin() noexcept;
	[[nodiscard]] const_iterator begin() const noexcept;
	[[nodiscard]] const_iterator cbegin() const noexcept;
	[[nodiscard]] iterator end() noexcept;
	[[nodiscard]] const_iterator end() const noexcept;
	[[nodiscard]] const_iterator cend() const noexcept;
	[[nodiscard]] reverse_iterator rbegin() noexcept;
	[[nodiscard]] const_reverse_iterator rbegin() const noexcept;
	[[nodiscard]] const_reverse_iterator crbegin() const noexcept;
	[[nodiscard]] reverse_iterator rend() noexcept;
	[[nodiscard]] const_reverse_iterator rend() const noexcept;
	[[nodiscard]] const_reverse_iterator crend() const noexcept;

	[[nodiscard]] bool empty() const noexcept;
	[[nodiscard]] size_type size() const noexcept;
	[[nodiscard]] size_type max_size() const noexcept;

	void clear() noexcept;

	template <typename P>
	std::pair<iterator, bool> insert(P&& value);

	template <typename P>
	iterator insert(const_iterator pos, P&& value);

	template <typename InputIt>
	void insert(InputIt first, InputIt last);
	void insert(std::initializer_list<value_type> ilist);

	template <typename... Args>
	std::pair<iterator, bool> emplace(Args&&... args);

	template <typename... Args>
	iterator emplace_hint(const_iterator hint, Args&&... args);

	template <typename... Args>
	std::pair<iterator, bool> try_emplace(const String& k, Args&&... args);

	template <typename... Args>
	std::pair<iterator, bool> try_emplace(String&& k, Args&&... args);

	template <typename... Args>
	iterator try_emplace(const_iterator, const String& k, Args&&... args);

	template <typename... Args>
	iterator try_emplace(const_iterator, String&& k, Args&&... args);

	iterator erase(const_iterator pos);
	size_type erase(std::string_view name);

	void swap(Object& other) noexcept;
	friend void swap(Object& a, Object& b) noexcept;

	[[nodiscard]] size_type count(std::string_view name) const noexcept;
	[[nodiscard]] bool contains(std::string_view name) const noexcept;
	[[nodiscard]] iterator find(std::string_view name) noexcept;
	[[nodiscard]] const_iterator find(std::string_view name) const noexcept;
	[[nodiscard]] std::pair<iterator, iterator> equal_range(std::string_view name) noexcept;
	[[nodiscard]] std::pair<const_iterator, const_iterator> equal_range(std::string_view name) const noexcept;
	[[nodiscard]] iterator lower_bound(std::string_view name) noexcept;
	[[nodiscard]] const_iterator lower_bound(std::string_view name) const noexcept;
	[[nodiscard]] iterator upper_bound(std::string_view name) noexcept;
	[[nodiscard]] const_iterator upper_bound(std::string_view name) const noexcept;

	[[nodiscard]] bool operator==(const Object& other) const noexcept;
	[[nodiscard]] std::partial_ordering operator<=>(const Object& other) const noexcept;

	template <typename Predicate>
	friend size_type erase_if(Object& container, Predicate predicate);

private:
	struct Compare {
		[[nodiscard]] bool operator()(const value_type& a, const value_type& b) const noexcept;
		[[nodiscard]] bool operator()(const value_type& a, std::string_view b) const noexcept;
		[[nodiscard]] bool operator()(std::string_view a, const value_type& b) const noexcept;
		[[nodiscard]] bool operator()(std::string_view a, std::string_view b) const noexcept;
	};

	std::vector<value_type> membersSortedByName;
};

/**
 * JSON array type whose API mimics that of std::vector<Value>.
 */
class Array {
public:
	using value_type = Value;
	using size_type = typename std::vector<value_type>::size_type;
	using difference_type = typename std::vector<value_type>::difference_type;
	using reference = typename std::vector<value_type>::reference;
	using const_reference = typename std::vector<value_type>::const_reference;
	using pointer = typename std::vector<value_type>::pointer;
	using const_pointer = typename std::vector<value_type>::const_pointer;
	using iterator = typename std::vector<value_type>::iterator;
	using const_iterator = typename std::vector<value_type>::const_iterator;
	using reverse_iterator = typename std::vector<value_type>::reverse_iterator;
	using const_reverse_iterator = typename std::vector<value_type>::const_reverse_iterator;

	Array() noexcept;
	~Array();

	template <typename InputIt>
	Array(InputIt first, InputIt last);
	Array(size_type count, const Value& value);
	Array(std::initializer_list<value_type> ilist);

	Array(const Array& other);
	Array(Array&& other) noexcept;
	Array& operator=(const Array& other);
	Array& operator=(Array&& other) noexcept;

	Array& operator=(std::initializer_list<value_type> ilist);

	void swap(Array& other) noexcept;
	friend void swap(Array& a, Array& b) noexcept;

	[[nodiscard]] pointer data() noexcept;
	[[nodiscard]] const_pointer data() const noexcept;
	[[nodiscard]] size_type size() const noexcept;
	[[nodiscard]] size_type max_size() const noexcept;
	[[nodiscard]] size_type capacity() const noexcept;
	[[nodiscard]] bool empty() const noexcept;

	[[nodiscard]] iterator begin() noexcept;
	[[nodiscard]] const_iterator begin() const noexcept;
	[[nodiscard]] const_iterator cbegin() const noexcept;
	[[nodiscard]] iterator end() noexcept;
	[[nodiscard]] const_iterator end() const noexcept;
	[[nodiscard]] const_iterator cend() const noexcept;
	[[nodiscard]] reverse_iterator rbegin() noexcept;
	[[nodiscard]] const_reverse_iterator rbegin() const noexcept;
	[[nodiscard]] const_reverse_iterator crbegin() const noexcept;
	[[nodiscard]] reverse_iterator rend() noexcept;
	[[nodiscard]] const_reverse_iterator rend() const noexcept;
	[[nodiscard]] const_reverse_iterator crend() const noexcept;

	[[nodiscard]] reference front();
	[[nodiscard]] const_reference front() const;
	[[nodiscard]] reference back();
	[[nodiscard]] const_reference back() const;
	[[nodiscard]] reference at(size_type pos);
	[[nodiscard]] const_reference at(size_type pos) const;
	[[nodiscard]] reference operator[](size_type pos);
	[[nodiscard]] const_reference operator[](size_type pos) const;

	[[nodiscard]] bool operator==(const Array& other) const;
	[[nodiscard]] std::partial_ordering operator<=>(const Array& other) const noexcept;

	template <typename U>
	friend size_type erase(Array& container, const U& value);

	template <typename Predicate>
	friend size_type erase_if(Array& container, Predicate predicate);

	void clear() noexcept;
	void reserve(size_type newCap);
	void shrink_to_fit();

	iterator insert(const_iterator pos, const Value& value);
	iterator insert(const_iterator pos, Value&& value);
	iterator insert(const_iterator pos, size_type count, const Value& value);

	template <typename InputIt>
	iterator insert(const_iterator pos, InputIt first, InputIt last);
	iterator insert(const_iterator pos, std::initializer_list<value_type> ilist);

	template <typename... Args>
	iterator emplace(const_iterator pos, Args&&... args);

	iterator erase(const_iterator pos);
	iterator erase(const_iterator first, const_iterator last);

	void push_back(const Value& value);
	void push_back(Value&& value);

	template <typename... Args>
	reference emplace_back(Args&&... args);

	void pop_back();

	void resize(size_type count);
	void resize(size_type count, const Value& value);

private:
	std::vector<value_type> values;
};

/**
 * JSON value type.
 *
 * Holds a value of one of the following types:
 * - Null
 * - Boolean
 * - String
 * - Number
 * - Object
 * - Array
 */
class Value : public Variant<Null, Boolean, String, Number, Object, Array> {
public:
	/**
	 * Parse a value of any JSON type from a UTF-8 JSON string.
	 *
	 * The parser supports JSON5 features such as comments, unquoted identifiers
	 * and trailing commas.
	 *
	 * \param jsonString read-only view over the JSON string to parse a value
	 *        from.
	 *
	 * \return the parsed value.
	 *
	 * \throws Error on failure to parse a JSON value.
	 * \throws std::bad_alloc on allocation failure.
	 */
	[[nodiscard]] static Value parse(std::u8string_view jsonString);

	/**
	 * Parse a value of any JSON type from a JSON string of bytes, interpreted
	 * as UTF-8.
	 *
	 * The parser supports JSON5 features such as comments, unquoted identifiers
	 * and trailing commas.
	 *
	 * \param jsonString read-only view over the JSON string to parse a value
	 *        from.
	 *
	 * \return the parsed value.
	 *
	 * \throws Error on failure to parse a JSON value.
	 * \throws std::bad_alloc on allocation failure.
	 */
	[[nodiscard]] static Value parse(std::string_view jsonString);

	/**
	 * Construct a Null value.
	 */
	Value() noexcept = default;

	/**
	 * Construct a Null value.
	 */
	Value(Null) noexcept {}

	/**
	 * Construct a Null value.
	 */
	Value(std::nullptr_t) noexcept {}

	/**
	 * Construct a Boolean value with the given underlying value.
	 *
	 * \param value value to copy.
	 */
	Value(Boolean value) noexcept
		: Variant(value) {}

	/**
	 * Construct a String value with the given underlying value.
	 *
	 * \param value value to copy.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 */
	Value(const String& value)
		: Variant(value) {}

	/**
	 * Construct a String value from the given underlying value.
	 *
	 * \param value value to take.
	 */
	Value(String&& value) noexcept
		: Variant(std::move(value)) {}

	/**
	 * Construct a String value with the given underlying value.
	 *
	 * \param value read-only pointer to the null-terminated string to copy the
	 *        contents of.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 */
	Value(const char* value)
		: Variant(String{value}) {}

	/**
	 * Construct a String value with the given underlying value.
	 *
	 * \param value read-only view over the string to copy the contents of.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 */
	Value(std::string_view value)
		: Variant(String{value}) {}

	/**
	 * Construct a String value with the given underlying value.
	 *
	 * \param value read-only pointer to the null-terminated UTF-8 string to
	 *        copy the contents of.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 */
	Value(const char8_t* value)
		: Variant(String{reinterpret_cast<const char*>(value)}) {
		static_assert(sizeof(char) == sizeof(char8_t));
		static_assert(alignof(char) == alignof(char8_t));
	}

	/**
	 * Construct a String value with the given underlying value.
	 *
	 * \param value read-only view over the UTF-8 string to copy the contents
	 *        of.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 */
	Value(std::u8string_view value)
		: Variant(String{value.begin(), value.end()}) {}

	/**
	 * Construct a Number value with the given underlying value.
	 *
	 * \param value value to copy. May be of any fundamental arithmetic type
	 *        except for bool and character types.
	 */
	Value(detail::number auto value) noexcept
		: Variant(static_cast<Number>(value)) {}

	/**
	 * Construct an Object value with the given underlying value.
	 *
	 * \param value value to copy.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 */
	Value(const Object& value)
		: Variant(value) {}

	/**
	 * Construct an Object value from the given underlying value.
	 *
	 * \param value value to take.
	 */
	Value(Object&& value) noexcept
		: Variant(std::move(value)) {}

	/**
	 * Construct an Array value with the given underlying value.
	 *
	 * \param value value to copy.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 */
	Value(const Array& value)
		: Variant(value) {}

	/**
	 * Construct an Array value from the given underlying value.
	 *
	 * \param value value to take.
	 */
	Value(Array&& value) noexcept
		: Variant(std::move(value)) {}

	/**
	 * Get a JSON string representation of the value.
	 * 
	 * \param options options for JSON serialization, see SerializationOptions.
	 *
	 * \return a JSON string containing a representation of the value as it
	 *         would be if it had been serialized to an output stream with the
	 *         given options.
	 *
	 * \throws Error on failure to serialize the value.
	 * \throws std::bad_alloc on allocation failure.
	 */
	[[nodiscard]] std::string toString(const SerializationOptions& options = {}) const;

	/**
	 * Compare this value to another for equality.
	 *
	 * \param other the value to compare this value to.
	 *
	 * \return true if the values are equal, false otherwise.
	 */
	[[nodiscard]] bool operator==(const Value& other) const {
		return static_cast<const Variant&>(*this) == static_cast<const Variant&>(other);
	}

	/**
	 * Compare this value to another.
	 *
	 * \param other the value to compare this value to.
	 *
	 * \return a partial ordering between the two values.
	 */
	[[nodiscard]] std::partial_ordering operator<=>(const Value& other) const {
		return std::compare_partial_order_fallback(static_cast<const Variant&>(*this), static_cast<const Variant&>(other));
	}
};

/**
 * Check if a Unicode code point is considered to be whitespace in JSON5.
 *
 * \param codePoint code point value to check.
 *
 * \return true if the code point is considered whitespace, false otherwise.
 */
[[nodiscard]] constexpr bool isWhitespaceCharacter(char32_t codePoint) noexcept {
	return codePoint == '\t' || codePoint == '\n' || codePoint == '\v' || codePoint == '\f' || codePoint == '\r' || codePoint == ' ' || codePoint == 0x00A0 ||
	       codePoint == 0x1680 || (codePoint >= 0x2000 && codePoint <= 0x200A) || codePoint == 0x2028 || codePoint == 0x2029 || codePoint == 0x202F || codePoint == 0x205F ||
	       codePoint == 0x3000 || codePoint == 0xFEFF;
}

/**
 * Check if a Unicode code point is considered to be punctuation in JSON5.
 *
 * \param codePoint code point value to check.
 *
 * \return true if the code point is considered punctuation, false otherwise.
 */
[[nodiscard]] constexpr bool isPunctuationCharacter(char32_t codePoint) noexcept {
	return codePoint == ',' || codePoint == ':' || codePoint == '[' || codePoint == ']' || codePoint == '{' || codePoint == '}';
}

/**
 * Check if a Unicode code point marks the beginning of a line terminator
 * sequence in JSON5.
 *
 * \param codePoint code point value to check.
 *
 * \return true if the code point is considered a line terminator, false
 *         otherwise.
 */
[[nodiscard]] constexpr bool isLineTerminatorCharacter(char32_t codePoint) noexcept {
	return codePoint == '\n' || codePoint == '\r' || codePoint == 0x2028 || codePoint == 0x2029;
}

/**
 * Type of a scanned JSON5 token.
 */
enum class TokenType : std::uint8_t {
	END_OF_FILE,                     ///< End-of-file marker.
	IDENTIFIER_NULL,                 ///< Keyword null.
	IDENTIFIER_FALSE,                ///< Keyword false.
	IDENTIFIER_TRUE,                 ///< Keyword true.
	IDENTIFIER_NAME,                 ///< Unquoted identifier, e.g. abc.
	PUNCTUATOR_COMMA,                ///< Comma ',' symbol.
	PUNCTUATOR_COLON,                ///< Colon ':' symbol.
	PUNCTUATOR_OPEN_SQUARE_BRACKET,  ///< Open square bracket '[' symbol.
	PUNCTUATOR_CLOSE_SQUARE_BRACKET, ///< Closing square bracket ']' symbol.
	PUNCTUATOR_OPEN_CURLY_BRACE,     ///< Open curly brace '{' symbol.
	PUNCTUATOR_CLOSE_CURLY_BRACE,    ///< Closing curly brace '}' symbol.
	STRING,                          ///< Quoted string literal, e.g. "abc".
	NUMBER_BINARY,                   ///< Binary number literal, e.g. 0b0000000111111111.
	NUMBER_OCTAL,                    ///< Octal number literal, e.g. 0777.
	NUMBER_DECIMAL,                  ///< Decimal number literal, e.g. 511.
	NUMBER_HEXADECIMAL,              ///< Hexadecimal number literal, e.g. 0x01FF.
	NUMBER_POSITIVE_INFINITY,        ///< Keyword Infinity.
	NUMBER_NEGATIVE_INFINITY,        ///< Keyword -Infinity.
	NUMBER_POSITIVE_NAN,             ///< Keyword NaN.
	NUMBER_NEGATIVE_NAN,             ///< Keyword -NaN.
};

/**
 * Token data scanned from JSON.
 */
struct Token {
	String string;         ///< Scanned string.
	SourceLocation source; ///< Location of the scanned string in the JSON source string.
	TokenType type;        ///< Scanned token type.
};

/**
 * Lexical analyzer for scanning and tokenizing input in the JSON5 format.
 *
 * \tparam It iterator type of the underlying input source. Must be an input
 *         iterator.
 */
template <typename It>
class Lexer {
public:
	/**
	 * Construct a lexer with a Unicode iterator pair as input.
	 *
	 * \param it iterator to the beginning of the JSON input to scan.
	 * \param end sentinel that marks the end of the JSON input to scan.
	 * \param source initial source location corresponding to the current
	 *               position of the iterator.
	 *
	 * \warning The iterator pair [it, end) must form a valid forward range.
	 */
	Lexer(unicode::UTF8Iterator<It> it, unicode::UTF8Sentinel end, const SourceLocation& source)
		: it(std::move(it))
		, end(end)
		, source(source) {}

	/**
	 * Scan and consume the next token from the input.
	 *
	 * This advances the internal state of the lexer.
	 *
	 * \return the scanned token.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 */
	Token scan() {
		skipWhitespace();
		if (hasReachedEnd()) {
			return {.string{}, .source = source, .type = TokenType::END_OF_FILE};
		}
		switch (peek()) {
			case '{': [[fallthrough]];
			case '}': [[fallthrough]];
			case '[': [[fallthrough]];
			case ']': [[fallthrough]];
			case ':': [[fallthrough]];
			case ',': return scanPunctuator();
			case '\"': [[fallthrough]];
			case '\'': return scanString();
			case '0': [[fallthrough]];
			case '1': [[fallthrough]];
			case '2': [[fallthrough]];
			case '3': [[fallthrough]];
			case '4': [[fallthrough]];
			case '5': [[fallthrough]];
			case '6': [[fallthrough]];
			case '7': [[fallthrough]];
			case '8': [[fallthrough]];
			case '9': [[fallthrough]];
			case '+': [[fallthrough]];
			case '-': [[fallthrough]];
			case '.': return scanNumber();
			default: return scanIdentifier();
		}
	}

private:
	void skipWhitespace() {
		while (!hasReachedEnd()) {
			if (isWhitespaceCharacter(peek())) {
				if (isLineTerminatorCharacter(peek())) {
					skipLineTerminatorSequence();
				} else {
					advance();
				}
			} else if (peek() == '/') {
				advance();
				if (hasReachedEnd()) {
					throw Error{"Invalid token.", source};
				}
				if (peek() == '/') {
					advance();
					while (!hasReachedEnd()) {
						if (isLineTerminatorCharacter(peek())) {
							skipLineTerminatorSequence();
							break;
						}
						advance();
					}
				} else if (peek() == '*') {
					advance();
					while (!hasReachedEnd()) {
						if (isLineTerminatorCharacter(peek())) {
							skipLineTerminatorSequence();
						} else if (peek() == '*') {
							advance();
							if (!hasReachedEnd() && peek() == '/') {
								advance();
								break;
							}
						} else {
							advance();
						}
					}
				} else {
					throw Error{"Invalid token.", source};
				}
			} else {
				break;
			}
		}
	}

	void skipLineTerminatorSequence() {
		if (peek() == '\r') {
			advance();
			if (!hasReachedEnd() && peek() == '\n') {
				advance();
			}
		} else {
			advance();
		}
		++source.lineNumber;
		source.columnNumber = 1;
	}

	void advance() {
		if (!currentCodePoint) {
			++it;
		}
		currentCodePoint.reset();
		++source.columnNumber;
	}

	[[nodiscard]] bool hasReachedEnd() const noexcept {
		return it == end && !currentCodePoint;
	}

	[[nodiscard]] char32_t peek() const {
		if (!currentCodePoint) {
			currentCodePoint = *it++;
		}
		return *currentCodePoint;
	}

	[[nodiscard]] std::optional<char32_t> lookahead() const {
		if (!currentCodePoint) {
			currentCodePoint = *it++;
		}
		if (it != end) {
			return *it;
		}
		return {};
	}

	[[nodiscard]] Token scanPunctuator() {
		String string{static_cast<char>(peek())};
		const SourceLocation punctuatorSource = source;
		TokenType type{};
		switch (peek()) {
			case ',': type = TokenType::PUNCTUATOR_COMMA; break;
			case ':': type = TokenType::PUNCTUATOR_COLON; break;
			case '[': type = TokenType::PUNCTUATOR_OPEN_SQUARE_BRACKET; break;
			case ']': type = TokenType::PUNCTUATOR_CLOSE_SQUARE_BRACKET; break;
			case '{': type = TokenType::PUNCTUATOR_OPEN_CURLY_BRACE; break;
			case '}': type = TokenType::PUNCTUATOR_CLOSE_CURLY_BRACE; break;
			default: break;
		}
		advance();
		return {.string = std::move(string), .source = punctuatorSource, .type = type};
	}

	[[nodiscard]] Token scanString() {
		const char32_t quoteCharacter = peek();
		String string{};
		const SourceLocation stringSource = source;
		advance();
		while (!hasReachedEnd()) {
			if (!unicode::isValidCodePoint(peek())) {
				throw Error{"Invalid UTF-8.", source};
			}
			if (peek() == quoteCharacter) {
				advance();
				return {.string = std::move(string), .source = stringSource, .type = TokenType::STRING};
			}
			if (isLineTerminatorCharacter(peek())) {
				throw Error{"Unexpected line terminator in string.", source};
			}
			if (peek() != '\\') {
				const unicode::EncodeUTF8FromCodePointResult codePointUTF8 = unicode::encodeUTF8FromCodePoint(peek());
				string.append(std::string_view{reinterpret_cast<const char*>(codePointUTF8.codeUnits.data()), codePointUTF8.size});
				advance();
				continue;
			}
			advance();
			if (hasReachedEnd()) {
				throw Error{"Empty escape sequence.", source};
			}
			if (isLineTerminatorCharacter(peek())) {
				skipLineTerminatorSequence();
				continue;
			}
			switch (peek()) {
				case '\"': string.push_back('\"'); break;
				case '\'': string.push_back('\''); break;
				case '\\': string.push_back('\\'); break;
				case 'b': string.push_back('\b'); break;
				case 'f': string.push_back('\f'); break;
				case 'n': string.push_back('\n'); break;
				case 'r': string.push_back('\r'); break;
				case 't': string.push_back('\t'); break;
				case 'v': string.push_back('\v'); break;
				case '0': [[fallthrough]];
				case '1': [[fallthrough]];
				case '2': [[fallthrough]];
				case '3': [[fallthrough]];
				case '4': [[fallthrough]];
				case '5': [[fallthrough]];
				case '6': [[fallthrough]];
				case '7': [[fallthrough]];
				case '8': [[fallthrough]];
				case '9': scanNumericEscapeSequence(string, 1, 3, 8, [](char32_t codePoint) noexcept -> bool { return (codePoint >= '0' && codePoint <= '7'); }); continue;
				case 'x':
					scanNumericEscapeSequence(string, 2, 2, 16, [](char32_t codePoint) noexcept -> bool {
						return (codePoint >= '0' && codePoint <= '9') || (codePoint >= 'a' && codePoint <= 'f') || (codePoint >= 'A' && codePoint <= 'F');
					});
					continue;
				case 'u':
					scanNumericEscapeSequence(string, 4, 4, 16, [](char32_t codePoint) noexcept -> bool {
						return (codePoint >= '0' && codePoint <= '9') || (codePoint >= 'a' && codePoint <= 'f') || (codePoint >= 'A' && codePoint <= 'F');
					});
					continue;
				case 'U':
					scanNumericEscapeSequence(string, 8, 8, 16, [](char32_t codePoint) noexcept -> bool {
						return (codePoint >= '0' && codePoint <= '9') || (codePoint >= 'a' && codePoint <= 'f') || (codePoint >= 'A' && codePoint <= 'F');
					});
					continue;
				default: {
					const unicode::EncodeUTF8FromCodePointResult codePointUTF8 = unicode::encodeUTF8FromCodePoint(peek());
					string.append(std::string_view{reinterpret_cast<const char*>(codePointUTF8.codeUnits.data()), codePointUTF8.size});
					break;
				}
			}
			advance();
		}
		throw Error{"Missing end of string quote character.", source};
	}

	[[nodiscard]] Token scanNumber() {
		String string{};
		const SourceLocation numberSource = source;
		bool negative = false;
		if (peek() == '+') {
			advance();
		} else if (peek() == '-') {
			string.push_back('-');
			advance();
			negative = true;
		}
		if (hasReachedEnd()) {
			throw Error{"Missing number.", source};
		}
		if (peek() == 'I') {
			if (scanIdentifier().type == TokenType::NUMBER_POSITIVE_INFINITY) {
				return {.string{}, .source = numberSource, .type = (negative) ? TokenType::NUMBER_NEGATIVE_INFINITY : TokenType::NUMBER_POSITIVE_INFINITY};
			}
			throw Error{"Invalid number.", numberSource};
		}
		if (peek() == 'N') {
			if (scanIdentifier().type == TokenType::NUMBER_POSITIVE_NAN) {
				return {.string{}, .source = numberSource, .type = (negative) ? TokenType::NUMBER_NEGATIVE_NAN : TokenType::NUMBER_POSITIVE_NAN};
			}
			throw Error{"Invalid number.", numberSource};
		}
		TokenType type = TokenType::NUMBER_DECIMAL;
		if (!hasReachedEnd() && peek() == '0') {
			string.push_back('0');
			advance();
			if (!hasReachedEnd() && (peek() == 'b' || peek() == 'B')) {
				string.push_back('b');
				advance();
				type = TokenType::NUMBER_BINARY;
			} else if (!hasReachedEnd() && (peek() == 'x' || peek() == 'X')) {
				string.push_back('x');
				advance();
				type = TokenType::NUMBER_HEXADECIMAL;
			} else if (hasReachedEnd() || peek() != '.') {
				type = TokenType::NUMBER_OCTAL;
			}
		}
		bool eNotation = false;
		bool fraction = false;
		while (!hasReachedEnd()) {
			if (peek() == '.') {
				if (lookahead() == '.') {
					break;
				}
				if (type != TokenType::NUMBER_DECIMAL) {
					break;
				}
				if (eNotation) {
					throw Error{"Decimal point in E notation exponent.", source};
				}
				if (fraction) {
					throw Error{"Multiple decimal points in number.", source};
				}
				string.push_back('.');
				advance();
				fraction = true;
			} else if ((peek() == 'e' || peek() == 'E') && type != TokenType::NUMBER_HEXADECIMAL) {
				if (type != TokenType::NUMBER_DECIMAL) {
					break;
				}
				if (eNotation) {
					throw Error{"Multiple exponent symbols in E notation.", source};
				}
				string.push_back('e');
				advance();
				eNotation = true;
				fraction = true;
				if (hasReachedEnd()) {
					throw Error{"Missing exponent in E notation.", source};
				}
				if (peek() >= '0' && peek() <= '9') {
					string.push_back(static_cast<char>(peek()));
					advance();
				} else if ((peek() == '+' || peek() == '-')) {
					string.push_back(static_cast<char>(peek()));
					advance();
					if (!hasReachedEnd() && peek() >= '0' && peek() <= '9') {
						string.push_back(static_cast<char>(peek()));
						advance();
					} else {
						throw Error{"Missing exponent in E notation.", source};
					}
				}
			} else if (                                                                    //
				(type == TokenType::NUMBER_BINARY && (peek() == '0' || peek() == '1')) ||  //
				(type == TokenType::NUMBER_OCTAL && (peek() >= '0' && peek() <= '7')) ||   //
				(type == TokenType::NUMBER_DECIMAL && (peek() >= '0' && peek() <= '9')) || //
				(type == TokenType::NUMBER_HEXADECIMAL && ((peek() >= '0' && peek() <= '9') || (peek() >= 'a' && peek() <= 'f') || (peek() >= 'A' && peek() <= 'F')))) {
				string.push_back(static_cast<char>(peek()));
				advance();
			} else if (peek() == '_') {
				advance();
			} else {
				break;
			}
		}
		if (!hasReachedEnd()) {
			if (!isWhitespaceCharacter(peek()) && !isPunctuationCharacter(peek()) && peek() != '\"' && peek() != '\'' && peek() != '/') {
				throw Error{"Invalid character after number.", source};
			}
		}
		return {.string = std::move(string), .source = numberSource, .type = type};
	}

	[[nodiscard]] Token scanIdentifier() {
		String string{};
		const SourceLocation identifierSource = source;
		do {
			if (!unicode::isValidCodePoint(peek())) {
				throw Error{"Invalid UTF-8.", source};
			}
			const unicode::EncodeUTF8FromCodePointResult codePointUTF8 = unicode::encodeUTF8FromCodePoint(peek());
			string.append(std::string_view{reinterpret_cast<const char*>(codePointUTF8.codeUnits.data()), codePointUTF8.size});
			advance();
		} while (!hasReachedEnd() && !isWhitespaceCharacter(peek()) && !isPunctuationCharacter(peek()) && peek() != '\"' && peek() != '\'' && peek() != '/');
		TokenType type = TokenType::IDENTIFIER_NAME;
		if (string == "null") {
			string = {};
			type = TokenType::IDENTIFIER_NULL;
		} else if (string == "false") {
			string = {};
			type = TokenType::IDENTIFIER_FALSE;
		} else if (string == "true") {
			string = {};
			type = TokenType::IDENTIFIER_TRUE;
		} else if (string == "Infinity") {
			string = {};
			type = TokenType::NUMBER_POSITIVE_INFINITY;
		} else if (string == "NaN") {
			string = {};
			type = TokenType::NUMBER_POSITIVE_NAN;
		}
		return {.string = std::move(string), .source = identifierSource, .type = type};
	}

	void scanNumericEscapeSequence(String& output, std::size_t minDigitCount, std::size_t maxDigitCount, int radix, bool (*isDigit)(char32_t) noexcept) {
		const SourceLocation escapeSequenceSource = source;
		std::string digits{};
		digits.reserve(maxDigitCount);
		while (digits.size() < maxDigitCount && !hasReachedEnd() && isDigit(peek())) {
			digits.push_back(static_cast<char>(peek()));
			advance();
		}
		if (digits.size() < minDigitCount) {
			throw Error{"Invalid escape sequence length.", escapeSequenceSource};
		}
		const char* const digitsBegin = digits.data();
		const char* const digitsEnd = digitsBegin + digits.size();
		std::uint32_t codePointValue = 0;
		if (const std::from_chars_result parseResult = std::from_chars(digitsBegin, digitsEnd, codePointValue, radix);
			parseResult.ec != std::errc{} || parseResult.ptr != digitsEnd || !unicode::isValidCodePoint(static_cast<char32_t>(codePointValue))) {
			throw Error{"Invalid code point value.", escapeSequenceSource};
		}
		const unicode::EncodeUTF8FromCodePointResult codePointUTF8 = unicode::encodeUTF8FromCodePoint(static_cast<char32_t>(codePointValue));
		output.append(std::string_view{reinterpret_cast<const char*>(codePointUTF8.codeUnits.data()), codePointUTF8.size});
	}

	mutable unicode::UTF8Iterator<It> it;
	unicode::UTF8Sentinel end;
	SourceLocation source;
	mutable std::optional<char32_t> currentCodePoint{};
};

/**
 * Syntactic analyzer for parsing input in the JSON5 format obtained from a
 * json::Lexer.
 *
 * \tparam It iterator type of the underlying input source. Must be an input
 *         iterator.
 */
template <typename It>
class Parser {
public:
	/**
	 * Polymorphic interface for visitation-based parsing of JSON values.
	 */
	class ValueVisitor {
	public:
		/**
		 * Callback for values of type Null.
		 *
		 * \param source location of the parsed value.
		 * \param value parsed value.
		 *
		 * \throws json::Error on invalid input.
		 * \throws any exception thrown by the concrete implementation.
		 */
		virtual void visitNull(const SourceLocation& source, Null value) {
			(void)value;
			throw Error{"Unexpected null.", source};
		}

		/**
		 * Callback for values of type Boolean.
		 *
		 * \param source location of the parsed value.
		 * \param value parsed value.
		 *
		 * \throws json::Error on invalid input.
		 * \throws any exception thrown by the concrete implementation.
		 */
		virtual void visitBoolean(const SourceLocation& source, Boolean value) {
			(void)value;
			throw Error{"Unexpected boolean.", source};
		}

		/**
		 * Callback for values of type String.
		 *
		 * \param source location of the parsed value.
		 * \param value parsed value.
		 *
		 * \throws json::Error on invalid input.
		 * \throws any exception thrown by the concrete implementation.
		 */
		virtual void visitString(const SourceLocation& source, String&& value) {
			(void)std::move(value);
			throw Error{"Unexpected string.", source};
		}

		/**
		 * Callback for values of type Number.
		 *
		 * \param source location of the parsed value.
		 * \param value parsed value.
		 *
		 * \throws json::Error on invalid input.
		 * \throws any exception thrown by the concrete implementation.
		 */
		virtual void visitNumber(const SourceLocation& source, Number value) {
			(void)value;
			throw Error{"Unexpected number.", source};
		}

		/**
		 * Callback for objects.
		 *
		 * \param source location of the beginning of the encountered object.
		 * \param parser parser that should be used to parse the object.
		 *
		 * \warning Implementations must advance the parser to the end of the
		 *          encountered object, past the last closing curly brace.
		 * \warning Implementations must not advance the parser past the end of
		 *          the encountered object.
		 *
		 * \throws json::Error on invalid input.
		 * \throws any exception thrown by the concrete implementation.
		 */
		virtual void visitObject(const SourceLocation& source, Parser& parser) {
			(void)parser;
			throw Error{"Unexpected object.", source};
		}

		/**
		 * Callback for arrays.
		 *
		 * \param source location of the beginning of the encountered array.
		 * \param parser parser that should be used to parse the array.
		 *
		 * \warning Implementations must advance the parser to the end of the
		 *          encountered array, past the last closing square bracket.
		 * \warning Implementations must not advance the parser past the end of
		 *          the encountered array.
		 *
		 * \throws json::Error on invalid input.
		 * \throws any exception thrown by the concrete implementation.
		 */
		virtual void visitArray(const SourceLocation& source, Parser& parser) {
			(void)parser;
			throw Error{"Unexpected array.", source};
		}

	protected:
		~ValueVisitor() = default;
	};

	/**
	 * Implementation of ValueVisitor for freestanding classes that implement
	 * all or parts of its interface without directly inheriting from it.
	 *
	 * \tparam Visitor freestanding value visitor type to adapt.
	 */
	template <typename Visitor>
	struct ConcreteValueVisitor final : ValueVisitor {
		Visitor visitor;

		ConcreteValueVisitor(Visitor visitor)
			: visitor(std::move(visitor)) {}

		void visitNull(const SourceLocation& source, Null value) override {
			if constexpr (requires { visitor.visitNull(source, value); }) {
				visitor.visitNull(source, value);
			} else {
				ValueVisitor::visitNull(source, value);
			}
		}

		void visitBoolean(const SourceLocation& source, Boolean value) override {
			if constexpr (requires { visitor.visitBoolean(source, value); }) {
				visitor.visitBoolean(source, value);
			} else {
				ValueVisitor::visitBoolean(source, value);
			}
		}

		void visitString(const SourceLocation& source, String&& value) override {
			if constexpr (requires { visitor.visitString(source, std::move(value)); }) {
				visitor.visitString(source, std::move(value));
			} else {
				ValueVisitor::visitString(source, std::move(value));
			}
		}

		void visitNumber(const SourceLocation& source, Number value) override {
			if constexpr (requires { visitor.visitNumber(source, value); }) {
				visitor.visitNumber(source, value);
			} else {
				ValueVisitor::visitNumber(source, value);
			}
		}

		void visitObject(const SourceLocation& source, Parser& parser) override {
			if constexpr (requires { visitor.visitObject(source, parser); }) {
				visitor.visitObject(source, parser);
			} else {
				ValueVisitor::visitObject(source, parser);
			}
		}

		void visitArray(const SourceLocation& source, Parser& parser) override {
			if constexpr (requires { visitor.visitArray(source, parser); }) {
				visitor.visitArray(source, parser);
			} else {
				ValueVisitor::visitArray(source, parser);
			}
		}
	};

	/**
	 * Polymorphic interface for visitation-based parsing of JSON object
	 * properties.
	 */
	class PropertyVisitor {
	public:
		/**
		 * Callback for each object property.
		 *
		 * \param source location of the beginning of the property's value.
		 * \param key the property's name string.
		 * \param parser parser that should be used to parse the property's
		 *        value.
		 *
		 * \warning Implementations must advance the parser to the end of the
		 *          encountered value.
		 * \warning Implementations must not advance the parser past the end of
		 *          the property's value.
		 *
		 * \throws json::Error on invalid input.
		 * \throws any exception thrown by the concrete implementation.
		 */
		virtual void visitProperty(const SourceLocation& source, String&& key, Parser& parser) = 0;

	protected:
		~PropertyVisitor() = default;
	};

	/**
	 * Implementation of PropertyVisitor for freestanding classes that implement
	 * all or parts of its interface without directly inheriting from it.
	 *
	 * \tparam Visitor freestanding property visitor type to adapt.
	 */
	template <typename Visitor>
	struct ConcretePropertyVisitor final : PropertyVisitor {
		Visitor visitor;

		ConcretePropertyVisitor(Visitor visitor)
			: visitor(std::move(visitor)) {}

		void visitProperty(const SourceLocation& source, String&& key, Parser& parser) override {
			if constexpr (requires { visitor.visitProperty(source, std::move(key), parser); }) {
				visitor.visitProperty(source, std::move(key), parser);
			}
		}
	};

	/**
	 * Implementation of ValueVisitor that skips over the parsed value and
	 * discards the result.
	 */
	struct SkipValueVisitor final : ValueVisitor {
		// clang-format off
		void visitNull(const SourceLocation& source, Null value) override { (void)source; (void)value; }
		void visitBoolean(const SourceLocation& source, Boolean value) override { (void)source; (void)value; }
		void visitString(const SourceLocation& source, String&& value) override { (void)source; (void)std::move(value); }
		void visitNumber(const SourceLocation& source, Number value) override { (void)source; (void)value; }
		void visitObject(const SourceLocation& source, Parser& parser) override { (void)source; parser.parseObject(SkipPropertyVisitor{}); }
		void visitArray(const SourceLocation& source, Parser& parser) override { (void)source; parser.parseValue(SkipValueVisitor{}); }
		// clang-format on
	};

	/**
	 * Implementation of PropertyVisitor that skips over the parsed property and
	 * discards the result.
	 */
	struct SkipPropertyVisitor final : PropertyVisitor {
		// clang-format off
		void visitProperty(const SourceLocation& source, String&& key, Parser& parser) override { (void)source; (void)std::move(key); parser.parseValue(SkipValueVisitor{}); }
		// clang-format on
	};

	/**
	 * Construct a parser with an existing lexer as input.
	 *
	 * \param lexer lexer to scan JSON tokens from.
	 */
	explicit Parser(Lexer<It> lexer)
		: lexer(std::move(lexer)) {}

	/**
	 * Construct a parser with a contiguous UTF-8 view as input.
	 *
	 * \param codePoints non-owning read-only view over the UTF-8 string to
	 *        parse JSON tokens from.
	 */
	explicit Parser(unicode::UTF8View codePoints) requires(std::is_same_v<It, const char8_t*>)
		: Parser(Lexer<It>{codePoints.begin(), codePoints.end(), SourceLocation{.lineNumber = 1, .columnNumber = 1}}) {}

	/**
	 * Construct a parser with a contiguous UTF-8 string as input.
	 *
	 * \param jsonString non-owning read-only view over the UTF-8 string to
	 *        parse JSON tokens from.
	 */
	explicit Parser(std::u8string_view jsonString) requires(std::is_same_v<It, const char8_t*>)
		: Parser(unicode::UTF8View{jsonString}) {}

	/**
	 * Construct a parser with a contiguous string of bytes, interpreted as
	 * UTF-8, as input.
	 *
	 * \param jsonString non-owning read-only view over the byte string to parse
	 *        JSON tokens from.
	 */
	explicit Parser(std::string_view jsonString) requires(std::is_same_v<It, const char8_t*>)
		: Parser(unicode::UTF8View{jsonString}) {}

	/**
	 * Construct a parser with an input stream as input.
	 *
	 * \param stream input stream to parse JSON tokens from.
	 */
	explicit Parser(std::istream& stream) requires(std::is_same_v<It, std::istreambuf_iterator<char>>)
		: Parser(Lexer<It>{unicode::UTF8Iterator<It>{It{stream}, It{}}, unicode::UTF8Sentinel{}, SourceLocation{.lineNumber = 1, .columnNumber = 1}}) {}

	/**
	 * Construct a parser with an input stream buffer as input.
	 *
	 * \param streambuf input stream buffer to parse JSON tokens from.
	 */
	explicit Parser(std::streambuf* streambuf) requires(std::is_same_v<It, std::istreambuf_iterator<char>>)
		: Parser(Lexer<It>{unicode::UTF8Iterator<It>{It{streambuf}, It{}}, unicode::UTF8Sentinel{}, SourceLocation{.lineNumber = 1, .columnNumber = 1}}) {}

	/**
	 * Read a single JSON value from the input and visit it, then make sure the
	 * rest of the input only consists of whitespace.
	 *
	 * \param visitor visitor to give the parsed value to.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 * \throws any exception thrown by the visitor.
	 *
	 * \sa json::onNull()
	 * \sa json::onBoolean()
	 * \sa json::onString()
	 * \sa json::onNumber()
	 * \sa json::onObject()
	 * \sa json::onArray()
	 * \sa parseFile()
	 * \sa parseValue(ValueVisitor&)
	 */
	void parseFile(ValueVisitor& visitor) {
		parseValue(visitor);
		if (const Token& token = peek(); token.type != TokenType::END_OF_FILE) {
			throw Error{"Multiple top-level values.", token.source};
		}
	}

	/**
	 * Read a single JSON value from the input and visit it.
	 *
	 * \param visitor visitor to give the parsed value to.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 * \throws any exception thrown by the visitor.
	 *
	 * \sa json::onNull()
	 * \sa json::onBoolean()
	 * \sa json::onString()
	 * \sa json::onNumber()
	 * \sa json::onObject()
	 * \sa json::onArray()
	 * \sa parseValue()
	 * \sa parseFile(ValueVisitor&)
	 */
	void parseValue(ValueVisitor& visitor) {
		switch (const Token& token = peek(); token.type) {
			case TokenType::END_OF_FILE: throw Error{"Expected a value.", token.source};
			case TokenType::IDENTIFIER_NULL:
				advance();
				visitor.visitNull(token.source, Null{});
				break;
			case TokenType::IDENTIFIER_FALSE:
				advance();
				visitor.visitBoolean(token.source, Boolean{false});
				break;
			case TokenType::IDENTIFIER_TRUE:
				advance();
				visitor.visitBoolean(token.source, Boolean{true});
				break;
			case TokenType::IDENTIFIER_NAME: throw Error{"Unexpected name identifier.", token.source};
			case TokenType::PUNCTUATOR_COMMA: throw Error{"Unexpected comma.", token.source};
			case TokenType::PUNCTUATOR_COLON: throw Error{"Unexpected colon.", token.source};
			case TokenType::PUNCTUATOR_OPEN_SQUARE_BRACKET: {
				const SourceLocation source = token.source;
				visitor.visitArray(source, *this);
				break;
			}
			case TokenType::PUNCTUATOR_CLOSE_SQUARE_BRACKET: throw Error{"Unexpected closing bracket.", token.source};
			case TokenType::PUNCTUATOR_OPEN_CURLY_BRACE: {
				const SourceLocation source = token.source;
				visitor.visitObject(source, *this);
				break;
			}
			case TokenType::PUNCTUATOR_CLOSE_CURLY_BRACE: throw Error{"Unexpected closing brace.", token.source};
			case TokenType::STRING: visitor.visitString(token.source, std::move(eat().string)); break;
			case TokenType::NUMBER_BINARY: visitor.visitNumber(token.source, parseNumberContents(eat(), 2)); break;
			case TokenType::NUMBER_OCTAL: visitor.visitNumber(token.source, parseNumberContents(eat(), 8)); break;
			case TokenType::NUMBER_DECIMAL: visitor.visitNumber(token.source, parseNumberContents(eat(), 10)); break;
			case TokenType::NUMBER_HEXADECIMAL: visitor.visitNumber(token.source, parseNumberContents(eat(), 16)); break;
			case TokenType::NUMBER_POSITIVE_INFINITY:
				advance();
				visitor.visitNumber(token.source, Number{std::numeric_limits<Number>::infinity()});
				break;
			case TokenType::NUMBER_NEGATIVE_INFINITY:
				advance();
				visitor.visitNumber(token.source, Number{-std::numeric_limits<Number>::infinity()});
				break;
			case TokenType::NUMBER_POSITIVE_NAN:
				advance();
				visitor.visitNumber(token.source, Number{std::numeric_limits<Number>::quiet_NaN()});
				break;
			case TokenType::NUMBER_NEGATIVE_NAN:
				advance();
				visitor.visitNumber(token.source, Number{-std::numeric_limits<Number>::quiet_NaN()});
				break;
		}
	}

	/**
	 * Read a single JSON object from the input and visit each of its
	 * properties.
	 *
	 * \param visitor visitor to give each parsed property of the object to.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 * \throws any exception thrown by the visitor.
	 *
	 * \sa json::onProperty()
	 * \sa parseObject()
	 */
	void parseObject(PropertyVisitor& visitor) {
		if (const Token& token = peek(); token.type != TokenType::PUNCTUATOR_OPEN_CURLY_BRACE) {
			throw Error{"Expected an object.", token.source};
		}
		advance();
		while (true) {
			String key{};
			switch (const Token& token = peek(); token.type) {
				case TokenType::END_OF_FILE: throw Error{"Missing end of object.", token.source};
				case TokenType::IDENTIFIER_NULL: throw Error{"Unexpected null.", token.source};
				case TokenType::IDENTIFIER_FALSE: throw Error{"Unexpected false.", token.source};
				case TokenType::IDENTIFIER_TRUE: throw Error{"Unexpected true.", token.source};
				case TokenType::IDENTIFIER_NAME: [[fallthrough]];
				case TokenType::STRING: key = std::move(eat().string); break;
				case TokenType::PUNCTUATOR_COMMA: [[fallthrough]];
				case TokenType::PUNCTUATOR_COLON: [[fallthrough]];
				case TokenType::PUNCTUATOR_OPEN_SQUARE_BRACKET: [[fallthrough]];
				case TokenType::PUNCTUATOR_CLOSE_SQUARE_BRACKET: [[fallthrough]];
				case TokenType::PUNCTUATOR_OPEN_CURLY_BRACE: throw Error{"Unexpected punctuator.", token.source};
				case TokenType::PUNCTUATOR_CLOSE_CURLY_BRACE: advance(); return;
				case TokenType::NUMBER_BINARY: [[fallthrough]];
				case TokenType::NUMBER_OCTAL: [[fallthrough]];
				case TokenType::NUMBER_DECIMAL: [[fallthrough]];
				case TokenType::NUMBER_HEXADECIMAL: [[fallthrough]];
				case TokenType::NUMBER_POSITIVE_INFINITY: [[fallthrough]];
				case TokenType::NUMBER_NEGATIVE_INFINITY: [[fallthrough]];
				case TokenType::NUMBER_POSITIVE_NAN: [[fallthrough]];
				case TokenType::NUMBER_NEGATIVE_NAN: throw Error{"Unexpected number.", token.source};
			}
			if (const Token token = eat(); token.type != TokenType::PUNCTUATOR_COLON) {
				throw Error{"Expected a colon.", token.source};
			}
			const SourceLocation source = peek().source;
			visitor.visitProperty(source, std::move(key), *this);
			if (peek().source == source) {
				struct SkipValue final : ValueVisitor {
					// clang-format off
					void visitNull(const SourceLocation& source, Null value) override { (void)source; (void)value; }
					void visitBoolean(const SourceLocation& source, Boolean value) override { (void)source; (void)value; }
					void visitString(const SourceLocation& source, String&& value) override { (void)source; (void)std::move(value); }
					void visitNumber(const SourceLocation& source, Number value) override { (void)source; (void)value; }
					void visitObject(const SourceLocation& source, Parser& parser) override { (void)source; (void)parser; }
					void visitArray(const SourceLocation& source, Parser& parser) override { (void)source; (void)parser; }
					// clang-format on
				};
				parseValue(SkipValue{});
			}
			if (const Token& token = peek(); token.type == TokenType::PUNCTUATOR_COMMA) {
				advance();
			} else if (token.type == TokenType::PUNCTUATOR_CLOSE_CURLY_BRACE) {
				advance();
				break;
			} else {
				throw Error{"Expected a comma or closing brace.", token.source};
			}
		}
	}

	/**
	 * Read a single JSON array from the input and visit each of its values.
	 *
	 * \param visitor visitor to give each parsed value of the array to.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 * \throws any exception thrown by the visitor.
	 *
	 * \sa json::onNull()
	 * \sa json::onBoolean()
	 * \sa json::onString()
	 * \sa json::onNumber()
	 * \sa json::onObject()
	 * \sa json::onArray()
	 * \sa parseArray()
	 */
	void parseArray(ValueVisitor& visitor) {
		if (const Token& token = peek(); token.type != TokenType::PUNCTUATOR_OPEN_SQUARE_BRACKET) {
			throw Error{"Expected an array.", token.source};
		}
		advance();
		while (true) {
			if (peek().type == TokenType::PUNCTUATOR_CLOSE_SQUARE_BRACKET) {
				advance();
				return;
			}
			parseValue(visitor);
			if (const Token& token = peek(); token.type == TokenType::PUNCTUATOR_COMMA) {
				advance();
			} else if (token.type == TokenType::PUNCTUATOR_CLOSE_SQUARE_BRACKET) {
				advance();
				break;
			} else {
				throw Error{"Expected a comma or closing bracket.", token.source};
			}
		}
	}

	/**
	 * \sa parseFile(ValueVisitor&)
	 */
	void parseFile(ValueVisitor&& visitor) { // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
		parseFile(visitor);
	}

	/**
	 * \sa parseValue(ValueVisitor&)
	 */
	void parseValue(ValueVisitor&& visitor) { // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
		parseValue(visitor);
	}

	/**
	 * \sa parseObject(PropertyVisitor&)
	 */
	void parseObject(PropertyVisitor&& visitor) { // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
		parseObject(visitor);
	}

	/**
	 * \sa parseArray(ValueVisitor&)
	 */
	void parseArray(ValueVisitor&& visitor) { // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
		parseArray(visitor);
	}

	/**
	 * \sa parseFile(ValueVisitor&)
	 */
	template <typename Visitor>
	void parseFile(Visitor visitor) {
		parseFile(static_cast<ValueVisitor&&>(ConcreteValueVisitor<Visitor>{std::move(visitor)}));
	}

	/**
	 * \sa parseValue(ValueVisitor&)
	 */
	template <typename Visitor>
	void parseValue(Visitor visitor) {
		parseValue(static_cast<ValueVisitor&&>(ConcreteValueVisitor<Visitor>{std::move(visitor)}));
	}

	/**
	 * \sa parseObject(PropertyVisitor&)
	 */
	template <typename Visitor>
	void parseObject(Visitor visitor) {
		parseObject(static_cast<PropertyVisitor&&>(ConcretePropertyVisitor<Visitor>{std::move(visitor)}));
	}

	/**
	 * \sa parseArray(ValueVisitor&)
	 */
	template <typename Visitor>
	void parseArray(Visitor visitor) {
		parseArray(static_cast<ValueVisitor&&>(ConcreteValueVisitor<Visitor>{std::move(visitor)}));
	}

	/**
	 * Parse a single JSON value from the input and discard the result, then
	 * make sure the rest of the input only consists of whitespace.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 *
	 * \sa parseFile()
	 * \sa skipValue()
	 */
	void skipFile() {
		parseFile(SkipValueVisitor{});
	}

	/**
	 * Parse a single JSON value from the input and discard the result.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 *
	 * \sa parseValue()
	 * \sa skipFile()
	 */
	void skipValue() {
		parseValue(SkipValueVisitor{});
	}

	/**
	 * Read a single JSON value from the input and make sure the rest of the
	 * input only consists of whitespace.
	 *
	 * \return the parsed value.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 *
	 * \sa parseFile(ValueVisitor&)
	 * \sa parseValue()
	 * \sa skipFile()
	 */
	Value parseFile() {
		Value result = parseValue();
		if (const Token& token = peek(); token.type != TokenType::END_OF_FILE) {
			throw Error{"Multiple top-level values.", token.source};
		}
		return result;
	}

	/**
	 * Read a single JSON value from the input.
	 *
	 * \return the parsed value.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 *
	 * \sa parseValue(ValueVisitor&)
	 * \sa parseFile()
	 * \sa parseNull()
	 * \sa parseBoolean()
	 * \sa parseString()
	 * \sa parseNumber()
	 * \sa parseObject()
	 * \sa parseArray()
	 * \sa skipValue()
	 */
	Value parseValue() {
		struct Visitor final : ValueVisitor {
			Value& result;

			explicit Visitor(Value& result) noexcept
				: result(result) {}

			// clang-format off
			void visitNull(const SourceLocation&, Null value) override { result = value; }
			void visitBoolean(const SourceLocation&, Boolean value) override { result = value; }
			void visitString(const SourceLocation&, String&& value) override { result = std::move(value); }
			void visitNumber(const SourceLocation&, Number value) override { result = value; }
			void visitObject(const SourceLocation&, Parser& parser) override { result = parser.parseObject(); }
			void visitArray(const SourceLocation&, Parser& parser) override { result = parser.parseArray(); }
			// clang-format on
		};
		Value result{};
		parseValue(Visitor{result});
		return result;
	}

	/**
	 * Read a single JSON value of type Null from the input.
	 *
	 * \return the parsed value.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 *
	 * \sa parseValue()
	 */
	Null parseNull() {
		const Token token = eat();
		switch (token.type) {
			case TokenType::IDENTIFIER_NULL: return Null{};
			default: break;
		}
		throw Error{"Expected a null.", token.source};
	}

	/**
	 * Read a single JSON value of type Boolean from the input.
	 *
	 * \return the parsed value.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 *
	 * \sa parseValue()
	 */
	Boolean parseBoolean() {
		const Token token = eat();
		switch (token.type) {
			case TokenType::IDENTIFIER_FALSE: return Boolean{false};
			case TokenType::IDENTIFIER_TRUE: return Boolean{true};
			default: break;
		}
		throw Error{"Expected a boolean.", token.source};
	}

	/**
	 * Read a single JSON value of type String from the input.
	 *
	 * \return the parsed value.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 *
	 * \sa parseValue()
	 */
	String parseString() {
		Token token = eat();
		switch (token.type) {
			case TokenType::STRING: return std::move(token.string);
			default: break;
		}
		throw Error{"Expected a string.", token.source};
	}

	/**
	 * Read a single JSON value of type Number from the input.
	 *
	 * \return the parsed value.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 *
	 * \sa parseValue()
	 */
	Number parseNumber() {
		Token token = eat();
		switch (token.type) {
			case TokenType::NUMBER_BINARY: return parseNumberContents(std::move(token), 2);
			case TokenType::NUMBER_OCTAL: return parseNumberContents(std::move(token), 8);
			case TokenType::NUMBER_DECIMAL: return parseNumberContents(std::move(token), 10);
			case TokenType::NUMBER_HEXADECIMAL: return parseNumberContents(std::move(token), 16);
			case TokenType::NUMBER_POSITIVE_INFINITY: return std::numeric_limits<Number>::infinity();
			case TokenType::NUMBER_NEGATIVE_INFINITY: return -std::numeric_limits<Number>::infinity();
			case TokenType::NUMBER_POSITIVE_NAN: return std::numeric_limits<Number>::quiet_NaN();
			case TokenType::NUMBER_NEGATIVE_NAN: return -std::numeric_limits<Number>::quiet_NaN();
			default: break;
		}
		throw Error{"Expected a number.", token.source};
	}

	/**
	 * Read a single JSON value of type Object from the input.
	 *
	 * \return the parsed value.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 *
	 * \sa parseObject(PropertyVisitor&)
	 * \sa parseValue()
	 */
	Object parseObject() {
		const Token& token = peek();
		switch (token.type) {
			case TokenType::PUNCTUATOR_OPEN_CURLY_BRACE: {
				struct Visitor final : PropertyVisitor {
					Object& result;

					explicit Visitor(Object& result) noexcept
						: result(result) {}

					void visitProperty(const SourceLocation&, String&& key, Parser& parser) override {
						result.emplace(std::move(key), std::move(parser.parseValue()));
					}
				};
				Object result{};
				parseObject(Visitor{result});
				return result;
			}
			default: break;
		}
		throw Error{"Expected an object.", token.source};
	}

	/**
	 * Read a single JSON value of type Array from the input.
	 *
	 * \return the parsed value.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 *
	 * \sa parseArray(ValueVisitor&)
	 * \sa parseValue()
	 */
	Array parseArray() {
		const Token& token = peek();
		switch (token.type) {
			case TokenType::PUNCTUATOR_OPEN_SQUARE_BRACKET: {
				Array result{};
				struct Visitor final : ValueVisitor {
					Array& result;

					explicit Visitor(Array& result) noexcept
						: result(result) {}

					// clang-format off
					void visitNull(const SourceLocation&, Null value) override { result.emplace_back(value); }
					void visitBoolean(const SourceLocation&, Boolean value) override { result.emplace_back(value); }
					void visitString(const SourceLocation&, String&& value) override { result.emplace_back(std::move(value)); }
					void visitNumber(const SourceLocation&, Number value) override { result.emplace_back(value); }
					void visitObject(const SourceLocation&, Parser& parser) override { result.emplace_back(parser.parseObject()); }
					void visitArray(const SourceLocation&, Parser& parser) override { result.emplace_back(parser.parseArray()); }
					// clang-format on
				};
				parseArray(Visitor{result});
				return result;
			}
			default: break;
		}
		throw Error{"Expected an array.", token.source};
	}

	/**
	 * Advance the internal state of the underlying lexer by one token.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 */
	void advance() {
		if (!currentToken) {
			lexer.scan();
		}
		currentToken.reset();
	}

	/**
	 * Peek the next token without advancing the internal state of the
	 * underlying lexer.
	 *
	 * \return a read-only reference to the next token to be read that is valid
	 *         until the next call to advance() or eat(), or until the parser is
	 *         moved from or destroyed, whichever happens first.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 *
	 * \warning Although it is const, this function is not thread-safe since it
	 *          mutates an internal lookahead buffer. Exclusive access is
	 *          therefore required for safety.
	 */
	[[nodiscard]] const Token& peek() const {
		if (!currentToken) {
			currentToken = lexer.scan();
		}
		return *currentToken;
	}

	/**
	 * Scan and consume the next token from the input.
	 *
	 * This advances the internal state of the underlying lexer.
	 *
	 * \return the scanned token.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 */
	[[nodiscard]] Token eat() {
		if (!currentToken) {
			currentToken = lexer.scan();
		}
		Token result = std::move(*currentToken);
		currentToken.reset();
		return result;
	}

private:
	[[nodiscard]] static Number parseNumberContents(Token token, int radix) {
		const char* numberStringBegin = token.string.c_str();
		char* const numberStringEnd = token.string.data() + token.string.size();
		char* endPointer = numberStringEnd;
		if (radix == 10) {
			const double number_value = std::strtod(numberStringBegin, &endPointer);
			if (endPointer != numberStringEnd) {
				throw Error{"Invalid number.", token.source};
			}
			return Number{number_value};
		}
		bool negative = false;
		if (!token.string.empty() && token.string.front() == '-') {
			negative = true;
			++numberStringBegin;
		}
		const unsigned long long integerNumberValue = std::strtoull(numberStringBegin, &endPointer, radix);
		if (endPointer != numberStringEnd) {
			throw Error{"Invalid number.", token.source};
		}
		const double numberValue = static_cast<double>(integerNumberValue);
		return Number{(negative) ? -numberValue : numberValue};
	}

	mutable Lexer<It> lexer;
	mutable std::optional<Token> currentToken{};
};

/**
 * Parser for reading contiguous UTF-8-encoded JSON strings.
 */
using StringParser = Parser<const char8_t*>;

/**
 * Parser for reading UTF-8-encoded JSON input stream buffers.
 */
using StreamParser = Parser<std::istreambuf_iterator<char>>;

namespace detail {

template <typename T, typename ObjectPropertyFilter = detail::AlwaysTrue, typename ArrayItemFilter = detail::AlwaysTrue>
[[nodiscard]] std::size_t getRecursiveSize(const T& value, ObjectPropertyFilter objectPropertyFilter, ArrayItemFilter arrayItemFilter);

} // namespace detail

/**
 * Base template to specialize in order to implement JSON serialization for a
 * specific type.
 *
 * The specialization should have a member function with one of the following
 * signatures:
 * ```
 * void serialize(Writer& writer, const T& value)
 * void serialize(Writer& writer, T value)
 * ```
 * where T is the specialized template type parameter.
 *
 * The implementation should write the output using the provided Writer.
 *
 * \tparam T the type that this serializer serializes.
 */
template <typename T>
struct Serializer;

/**
 * Base template to specialize in order to implement JSON deserialization for a
 * specific type.
 *
 * The specialization should have a member function with the following
 * signature:
 * ```
 * void deserialize(Reader& reader, T& value)
 * ```
 * where T is the specialized template type parameter.
 *
 * The implementation should read the input using the provided Reader.
 *
 * \tparam T the type that this deserializer deserializes.
 */
template <typename T>
struct Deserializer;

/**
 * Serialize a value of any JSON-serializable type to an output stream.
 *
 * \param stream stream to write the output to.
 * \param value value to serialize.
 * \param options serialization options, see SerializationOptions.
 *
 * \throws Error on failure to serialize the value.
 * \throws std::ios_base::failure if thrown by the output stream.
 * \throws std::bad_alloc on allocation failure.
 * \throws any exception thrown by a user-defined implementation of Serializer,
 *         if one is used in the serialization of the given value type.
 *
 * \note Serialization of user-defined types can be defined by implementing a
 *       specialization of the Serializer template.
 */
template <typename T>
void serialize(std::ostream& stream, const T& value, const SerializationOptions& options = {});

/**
 * Deserialize a value of any JSON-serializable type from an input stream.
 *
 * \param stream stream to read the input from.
 * \param value value to deserialize to.
 * \param options deserialization options, see DeserializationOptions.
 *
 * \throws Error on failure to parse the value from the stream.
 * \throws std::ios_base::failure if thrown by the input stream.
 * \throws std::bad_alloc on allocation failure.
 * \throws any exception thrown by a user-defined implementation of
 *         Deserializer, if one is used in the deserialization of the given
 *         value type.
 *
 * \note Deserialization of user-defined types can be defined by implementing a
 *       specialization of the Deserializer template.
 */
template <typename T>
void deserialize(std::istream& stream, T& value, const DeserializationOptions& options = {});

/**
 * Write a JSON value to an output stream using the default serialization
 * options.
 *
 * \param stream stream to write the output to.
 * \param value the JSON value to serialize.
 *
 * \return a reference to the stream parameter, for chaining.
 *
 * \throws Error on failure to serialize the value.
 * \throws std::ios_base::failure if thrown by the output stream.
 * \throws std::bad_alloc on allocation failure.
 */
inline std::ostream& operator<<(std::ostream& stream, const Value& value) {
	json::serialize(stream, value);
	return stream;
}

/**
 * Read a JSON value from an input stream using the default deserialization
 * options.
 *
 * If the function fails to parse a JSON value from the stream,
 * std::ios_base::failbit is set on the stream, which may or may not
 * cause an exception to be thrown.
 *
 * \param stream stream to read the input from.
 * \param value the JSON value to deserialize to.
 *
 * \return a reference to the stream parameter, for chaining.
 *
 * \throws std::ios_base::failure if thrown by the input stream.
 * \throws std::bad_alloc on allocation failure.
 */
inline std::istream& operator>>(std::istream& stream, Value& value) {
	try {
		json::deserialize(stream, value);
	} catch (const Error&) {
		stream.setstate(std::istream::failbit);
	}
	return stream;
}

/**
 * Stateful wrapper object of an output stream for JSON serialization.
 */
struct Writer {
private:
	std::ostream& stream;

public:
	/**
	 * The current options of the serialization process.
	 */
	SerializationOptions options;

	/**
	 * Construct a writer with an output stream as output.
	 *
	 * \param stream output stream to write to.
	 * \param options output options, see SerializationOptions.
	 */
	Writer(std::ostream& stream, const SerializationOptions& options = {})
		: stream(stream)
		, options(options) {}

	/**
	 * Write a single raw byte to the output without any extra formatting.
	 *
	 * \param byte value to write.
	 *
	 * \throws any exception thrown by the underlying output stream.
	 */
	void write(char byte) {
		stream << byte;
	}

	/**
	 * Write a raw sequence of bytes to the output without any extra formatting.
	 *
	 * \param bytes values to write.
	 *
	 * \throws any exception thrown by the underlying output stream.
	 */
	void write(std::string_view bytes) {
		stream << bytes;
	}

	/**
	 * Write a sequence of indentation characters to the output.
	 *
	 * The length of the sequence matches the current indentation level
	 * specified in the options.
	 *
	 * \throws any exception thrown by the underlying output stream.
	 *
	 * \sa SerializationOptions::indentation
	 * \sa SerializationOptions::indentationCharacter
	 */
	void writeIndentation() {
		for (std::size_t i = 0; i < options.indentation; ++i) {
			write(options.indentationCharacter);
		}
	}

	/**
	 * Write a raw CRLF newline sequence to the output.
	 *
	 * \throws any exception thrown by the underlying output stream.
	 */
	void writeNewline() {
		write("\r\n");
	}

	/**
	 * Write a single JSON value of type Null to the output.
	 *
	 * \throws any exception thrown by the underlying output stream.
	 */
	void writeNull() {
		write("null");
	}

	/**
	 * Write a single JSON value of type Boolean to the output.
	 *
	 * \param value value to write.
	 *
	 * \throws any exception thrown by the underlying output stream.
	 */
	void writeBoolean(Boolean value) {
		write((value) ? "true" : "false");
	}

	/**
	 * Write a single JSON value of type String to the output from a raw byte
	 * string.
	 *
	 * \param bytes raw string value to write.
	 *
	 * \throws any exception thrown by the underlying output stream.
	 */
	void writeString(std::string_view bytes) {
		constexpr std::array<char, 16> HEXADECIMAL_DIGITS{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
		write('\"');
		for (const char byte : bytes) {
			if (byte >= ' ' && byte <= '~' && byte != '\"' && byte != '\\') {
				write(byte);
			} else {
				write('\\');
				switch (byte) {
					case '\"': write('\"'); break;
					case '\\': write('\\'); break;
					case '\b': write('b'); break;
					case '\f': write('f'); break;
					case '\n': write('n'); break;
					case '\r': write('r'); break;
					case '\t': write('t'); break;
					case '\v': write('v'); break;
					case '\0': write('0'); break;
					default:
						write('x');
						write(HEXADECIMAL_DIGITS[(byte >> 4) & 0x0F]);
						write(HEXADECIMAL_DIGITS[(byte & 0x0F)]);
						break;
				}
			}
		}
		write('\"');
	}

	/**
	 * Write a single JSON value of type String to the output from a raw string,
	 * interpreted as raw bytes.
	 *
	 * \param value raw string value whose bytes to write.
	 *
	 * \throws any exception thrown by the underlying output stream.
	 */
	template <typename CharT, typename Traits>
	void writeString(std::basic_string_view<CharT, Traits> value) requires(!std::is_same_v<CharT, char> || !std::is_same_v<Traits, std::char_traits<char>>) {
		const std::span<const std::byte> bytes = std::as_bytes(std::span{value.data(), value.size()});
		writeString(std::string_view{reinterpret_cast<const char*>(bytes.data()), bytes.size()});
	}

	/**
	 * Write a single JSON value of type String to the output from any
	 * string-view-like or JSON-serializable value.
	 *
	 * \param value value to write.
	 *
	 * \throws any exception thrown by the underlying output stream.
	 * \throws any exception thrown by the underlying string view conversion or
	 *         Serializer implementation of the given value type.
	 */
	void writeString(const auto& value) {
		if constexpr (requires { std::string_view{value}; }) {
			writeString(std::string_view{value});
		} else if constexpr (requires { std::u8string_view{value}; }) {
			writeString(std::u8string_view{value});
		} else if constexpr (requires { std::u16string_view{value}; }) {
			writeString(std::u16string_view{value});
		} else if constexpr (requires { std::u32string_view{value}; }) {
			writeString(std::u32string_view{value});
		} else if constexpr (requires { std::wstring_view{value}; }) {
			writeString(std::wstring_view{value});
		} else {
			std::ostringstream stringStream{};
			json::serialize(stringStream, value, {.prettyPrint = false});
			writeString(std::move(stringStream).str());
		}
	}

	/**
	 * Write a single JSON value of type Number to the output.
	 *
	 * \param value value to write.
	 *
	 * \throws any exception thrown by the underlying output stream.
	 */
	void writeNumber(Number value) {
		if (std::isnan(value)) {
			if (std::signbit(value)) {
				stream << "-NaN";
			} else {
				stream << "NaN";
			}
		} else if (std::isinf(value)) {
			if (std::signbit(value)) {
				stream << "-Infinity";
			} else {
				stream << "Infinity";
			}
		} else {
			[[likely]] fmt::format_to(std::ostreambuf_iterator{stream}, "{}", value);
		}
	}

	/**
	 * Write a single JSON object to the output from any range of
	 * JSON-serializable key-value pairs.
	 *
	 * \param value range to write as an object.
	 * \param propertyFilter predicate that each key-value pair from the range
	 *        must pass in order to be included in the output. Defaults to
	 *        always pass.
	 * \param getKey function for getting the key from each key-value pair.
	 *        Defaults to returning pair.first.
	 * \param getValue function for getting the value from each key-value pair.
	 *        Defaults to returning pair.second.
	 *
	 * \throws any exception thrown by the underlying output stream.
	 * \throws any exception thrown by the Serializer implementations of the
	 *         given key/value types.
	 */
	template <typename PropertyFilter = detail::AlwaysTrue, typename GetKey = detail::GetFirst, typename GetValue = detail::GetSecond>
	void writeObject(const auto& value, PropertyFilter propertyFilter = {}, GetKey getKey = {}, GetValue getValue = {}) {
		auto it = std::begin(value);
		const auto end = std::end(value);
		while (it != end && !propertyFilter(*it)) {
			++it;
		}
		if (options.prettyPrint) {
			if (it == end) {
				write("{}");
			} else if (detail::getRecursiveSize(value, propertyFilter, {}) - 1 <= options.prettyPrintMaxSingleLineObjectPropertyCount) {
				write("{ ");
				writeString(getKey(*it));
				write(": ");
				serialize(getValue(*it));
				for (++it; it != end; ++it) {
					if (propertyFilter(*it)) {
						write(", ");
						writeString(getKey(*it));
						write(": ");
						serialize(getValue(*it));
					}
				}
				write(" }");
			} else {
				write('{');
				writeNewline();
				options.indentation += options.relativeIndentation;
				writeIndentation();
				writeString(getKey(*it));
				write(": ");
				serialize(getValue(*it));
				for (++it; it != end; ++it) {
					if (propertyFilter(*it)) {
						write(',');
						writeNewline();
						writeIndentation();
						writeString(getKey(*it));
						write(": ");
						serialize(getValue(*it));
					}
				}
				writeNewline();
				options.indentation -= options.relativeIndentation;
				writeIndentation();
				write('}');
			}
		} else {
			write('{');
			if (it != end) {
				writeString(getKey(*it));
				write(':');
				serialize(getValue(*it));
				for (++it; it != end; ++it) {
					if (propertyFilter(*it)) {
						write(',');
						writeString(getKey(*it));
						write(':');
						serialize(getValue(*it));
					}
				}
			}
			write('}');
		}
	}

	/**
	 * Write a single JSON array to the output from any range of
	 * JSON-serializable values.
	 *
	 * \param value range to write as an array.
	 * \param itemFilter predicate that each value from the range must pass in
	 *        order to be included in the output. Defaults to always pass.
	 * \param getValue function for getting the value from each value of the
	 *        range. Defaults to returning each value without modification.
	 *
	 * \throws any exception thrown by the underlying output stream.
	 * \throws any exception thrown by the Serializer implementation of the
	 *         element type of the given value type.
	 */
	template <typename ItemFilter = detail::AlwaysTrue, typename GetValue = detail::Get>
	void writeArray(const auto& value, ItemFilter itemFilter = {}, GetValue getValue = {}) {
		auto it = std::begin(value);
		const auto end = std::end(value);
		while (it != end && !itemFilter(*it)) {
			++it;
		}
		if (options.prettyPrint) {
			if (it == end) {
				write("[]");
			} else if (detail::getRecursiveSize(value, {}, itemFilter) - 1 <= options.prettyPrintMaxSingleLineArrayItemCount) {
				write('[');
				serialize(getValue(*it));
				for (++it; it != end; ++it) {
					if (itemFilter(*it)) {
						write(", ");
						serialize(getValue(*it));
					}
				}
				write(']');
			} else {
				write('[');
				writeNewline();
				options.indentation += options.relativeIndentation;
				writeIndentation();
				serialize(getValue(*it));
				for (++it; it != end; ++it) {
					if (itemFilter(*it)) {
						write(',');
						writeNewline();
						writeIndentation();
						serialize(getValue(*it));
					}
				}
				writeNewline();
				options.indentation -= options.relativeIndentation;
				writeIndentation();
				write(']');
			}
		} else {
			write('[');
			if (it != end) {
				serialize(getValue(*it));
				for (++it; it != end; ++it) {
					if (itemFilter(*it)) {
						write(',');
						serialize(getValue(*it));
					}
				}
			}
			write(']');
		}
	}

	/**
	 * Write a single JSON value to the output from any value that supports
	 * conversion to bool and the dereference operator.
	 *
	 * If the value evaluates to true when converted to bool, the value is
	 * dereferenced and serialized normally. Otherwise, null is written.
	 *
	 * \param value value to write.
	 *
	 * \throws any exception thrown by the underlying output stream.
	 * \throws any exception thrown by the Serializer implementation of the
	 *         dereferenced type of the given value type.
	 */
	void writeOptional(const auto& value) {
		if (value) {
			serialize(*value);
		} else {
			writeNull();
		}
	}

	/**
	 * Write a single JSON value to the output from any value of aggregate type.
	 *
	 * \param value aggregate whose fields to write.
	 *
	 * \throws any exception thrown by the underlying output stream.
	 * \throws any exception thrown by the Serializer implementations of the
	 *         given field types.
	 */
	template <typename T>
	void writeAggregate(const T& value) {
		if constexpr (reflection::aggregate_size_v<T> == 0) {
			write("[]");
		} else if constexpr (reflection::aggregate_size_v<T> == 1) {
			const auto& [v] = value;
			serialize(v);
		} else if (options.prettyPrint) {
			if (detail::getRecursiveSize(value, {}, {}) - 1 <= options.prettyPrintMaxSingleLineArrayItemCount) {
				write('[');
				bool successor = false;
				reflection::forEach(reflection::fields(value), [&](const auto& v) {
					if (successor) {
						write(", ");
					}
					successor = true;
					serialize(v);
				});
				write(']');
			} else {
				write('[');
				writeNewline();
				options.indentation += options.relativeIndentation;
				bool successor = false;
				reflection::forEach(reflection::fields(value), [&](const auto& v) {
					if (successor) {
						write(',');
						writeNewline();
					}
					successor = true;
					writeIndentation();
					serialize(v);
				});
				writeNewline();
				options.indentation -= options.relativeIndentation;
				writeIndentation();
				write(']');
			}
		} else {
			write('[');
			bool successor = false;
			reflection::forEach(reflection::fields(value), [&](const auto& v) {
				if (successor) {
					write(',');
				}
				successor = true;
				serialize(v);
			});
			write(']');
		}
	}

	/**
	 * Write any JSON-serializable value to the output using its corresponding
	 * implementation of Serializer.
	 *
	 * \param value value to write.
	 *
	 * \throws any exception thrown by the underlying output stream.
	 * \throws any exception thrown by the Serializer implementation of T.
	 */
	template <typename T>
	void serialize(const T& value) {
		json::Serializer<std::remove_cvref_t<T>>{}.serialize(*this, value);
	}
};

/**
 * Stateful wrapper object of an input stream for JSON deserialization.
 */
struct Reader {
private:
	using It = std::istreambuf_iterator<char>;

	Parser<It> parser;

public:
	/**
	 * The current options of the deserialization process.
	 */
	DeserializationOptions options;

	/**
	 * Construct a reader with an input stream as input.
	 *
	 * \param stream input stream to write to.
	 * \param options input options, see DeserializationOptions.
	 */
	Reader(std::istream& stream, const DeserializationOptions& options)
		: parser(stream)
		, options(options) {}

	/**
	 * Read a single JSON value of type Null from the input.
	 *
	 * \return the location of the parsed value.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input stream.
	 *
	 * \sa Parser::parseNull()
	 * \sa readValue()
	 */
	SourceLocation readNull() {
		const SourceLocation source = parser.peek().source;
		parser.parseNull();
		return source;
	}

	/**
	 * Read a single JSON value of type Boolean from the input.
	 *
	 * \param value reference to the output value to write the parsed result to.
	 *
	 * \return the location of the parsed value.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input stream.
	 *
	 * \note If any exception is thrown, the output value is left unmodified.
	 *
	 * \sa Parser::parseBoolean()
	 */
	SourceLocation readBoolean(Boolean& value) {
		const SourceLocation source = parser.peek().source;
		value = parser.parseBoolean();
		return source;
	}

	/**
	 * Read a single JSON value of type String from the input.
	 *
	 * \param value reference to the output value to write the parsed result to.
	 *
	 * \return the location of the parsed value.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input stream.
	 *
	 * \note If any exception is thrown, the output value is left unmodified.
	 *
	 * \sa Parser::parseString()
	 */
	SourceLocation readString(String& value) {
		const SourceLocation source = parser.peek().source;
		value = parser.parseString();
		return source;
	}

	/**
	 * Read a single JSON value of type String from the input into any value
	 * that can be assigned from a standard string or deserialized from JSON.
	 *
	 * \param value reference to the output value to write the parsed result to.
	 *
	 * \return the location of the parsed value.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 * \throws any exception thrown by the underlying assignment operator or
	 *         Deserializer implementation of the given value type.
	 */
	SourceLocation readString(auto& value) {
		String string{};
		const SourceLocation source = readString(string);
		if constexpr (requires { value = std::move(string); }) {
			value = std::move(string);
		} else if constexpr (requires { value = std::string{}; }) {
			value = std::string{std::move(string)};
		} else if constexpr (requires { value = std::u8string{}; }) {
			value = std::u8string{string.begin(), string.end()};
		} else if constexpr (requires { value = std::u16string{}; }) {
			if (string.size() % sizeof(char16_t) != 0) {
				throw Error{"Expected a sequence of complete UTF-16 code units.", source};
			}
			std::u16string temporaryString = std::u16string(string.size() / sizeof(char16_t), char16_t{});
			std::memcpy(temporaryString.data(), string.data(), string.size());
			value = std::move(temporaryString);
		} else if constexpr (requires { value = std::u32string{}; }) {
			if (string.size() % sizeof(char32_t) != 0) {
				throw Error{"Expected a sequence of complete UTF-32 code units.", source};
			}
			std::u32string temporaryString = std::u32string(string.size() / sizeof(char32_t), char32_t{});
			std::memcpy(temporaryString.data(), string.data(), string.size());
			value = std::move(temporaryString);
		} else if constexpr (requires { value = std::wstring{}; }) {
			if (string.size() % sizeof(wchar_t) != 0) {
				throw Error{"Expected a sequence of complete wide characters.", source};
			}
			std::wstring temporaryString = std::wstring(string.size() / sizeof(wchar_t), wchar_t{});
			std::memcpy(temporaryString.data(), string.data(), string.size());
			value = std::move(temporaryString);
		} else {
			std::istringstream stringStream{std::move(string)}; // NOLINT(performance-move-const-arg)
			json::deserialize(stringStream, value);
		}
		return source;
	}

	/**
	 * Read a single JSON value of type Number from the input.
	 *
	 * \param value reference to the output value to write the parsed result to.
	 *
	 * \return the location of the parsed value.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input stream.
	 *
	 * \note If any exception is thrown, the output value is left unmodified.
	 *
	 * \sa Parser::parseNumber()
	 */
	SourceLocation readNumber(Number& value) {
		const SourceLocation source = parser.peek().source;
		value = parser.parseNumber();
		return source;
	}

	/**
	 * Read a single JSON value of type Number from the input into any value
	 * that Number can be explicitly converted to.
	 *
	 * \param value reference to the output value to write the parsed result to.
	 *
	 * \return the location of the parsed value.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 * \throws any exception thrown by the underlying conversion or assignment
	 *         operator of the given value type.
	 */
	template <typename T>
	SourceLocation readNumber(T& value) {
		Number number{};
		const SourceLocation source = readNumber(number);
		value = static_cast<T>(number);
		return source;
	}

	/**
	 * Read a single JSON value of type Object from the input.
	 *
	 * \param value reference to the output value to write the parsed result to.
	 *
	 * \return the location of the parsed value.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input stream.
	 *
	 * \note If any exception is thrown, the output value is left unmodified.
	 *
	 * \sa Parser::parseObject()
	 */
	SourceLocation readObject(Object& value) {
		const SourceLocation source = parser.peek().source;
		value = parser.parseObject();
		return source;
	}

	/**
	 * Read a single JSON object from the input into any container of key-value
	 * pairs where the key and value types are default-constructible and
	 * deserializable from JSON, and where the container supports `clear()` and
	 * `emplace(std::move(key), std::move(value))`.
	 *
	 * \param value reference to the output container to write the parsed results
	 *        into.
	 *
	 * \return the location of the beginning of the parsed object.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 * \throws any exception thrown by the default constructors of the given
	 *         key/value types.
	 * \throws any exception thrown by the Deserializer implementations of the
	 *         given key/value types.
	 * \throws any exception thrown by `clear()` or
	 *         `emplace(std::move(key), std::move(value))`.
	 *
	 * \warning If an exception is thrown, the output value may be left empty or
	 *          with some successfully parsed properties added to it, since they
	 *          are not removed automatically if a later operation fails.
	 */
	SourceLocation readObject(auto& value) {
		const SourceLocation source = parser.peek().source;
		if (const Token token = parser.eat(); token.type != TokenType::PUNCTUATOR_OPEN_CURLY_BRACE) {
			throw Error{"Expected an object.", token.source};
		}
		value.clear();
		if (parser.peek().type != TokenType::PUNCTUATOR_CLOSE_CURLY_BRACE) {
			while (true) {
				std::remove_cvref_t<decltype(std::begin(value)->first)> propertyKey{};
				std::remove_cvref_t<decltype(std::begin(value)->second)> propertyValue{};
				readString(propertyKey);
				if (const Token token = parser.eat(); token.type != TokenType::PUNCTUATOR_COLON) {
					throw Error{"Expected a colon.", token.source};
				}
				deserialize(propertyValue);
				value.emplace(std::move(propertyKey), std::move(propertyValue));
				const Token token = parser.eat();
				if (token.type == TokenType::PUNCTUATOR_CLOSE_CURLY_BRACE) {
					break;
				}
				if (token.type == TokenType::PUNCTUATOR_COMMA) {
					if (parser.peek().type == TokenType::PUNCTUATOR_CLOSE_CURLY_BRACE) {
						parser.advance();
						break;
					}
				} else {
					throw Error{"Expected a comma or closing brace.", token.source};
				}
			}
		}
		return source;
	}

	/**
	 * Read a single JSON value of type Array from the input.
	 *
	 * \param value reference to the output value to write the parsed result to.
	 *
	 * \return the location of the parsed value.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input stream.
	 *
	 * \note If any exception is thrown, the output value is left unmodified.
	 *
	 * \sa Parser::parseArray()
	 */
	SourceLocation readArray(Array& value) {
		const SourceLocation source = parser.peek().source;
		value = parser.parseArray();
		return source;
	}

	/**
	 * Read a single JSON array from the input into any container of elements
	 * that are default-constructible and deserializable from JSON, where the
	 * container supports `clear()` and `push_back(std::move(element))`.
	 *
	 * \param value reference to the output container to write the parsed results
	 *        into.
	 *
	 * \return the location of the beginning of the parsed array.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 * \throws any exception thrown by the default constructor of the given
	 *         element type.
	 * \throws any exception thrown by the Deserializer implementation of the
	 *         given element type.
	 * \throws any exception thrown by `clear()` or `push_back(std::move(element))`.
	 *
	 * \warning If an exception is thrown, the output value may be left empty or
	 *          with some successfully parsed items added to it, since they are
	 *          not removed automatically if a later operation fails.
	 */
	SourceLocation readArray(auto& value) {
		const SourceLocation source = parser.peek().source;
		if (const Token token = parser.eat(); token.type != TokenType::PUNCTUATOR_OPEN_SQUARE_BRACKET) {
			throw Error{"Expected an array.", token.source};
		}
		value.clear();
		if (parser.peek().type != TokenType::PUNCTUATOR_CLOSE_SQUARE_BRACKET) {
			while (true) {
				std::remove_cvref_t<decltype(*std::begin(value))> item{};
				deserialize(item);
				value.push_back(std::move(item));
				const Token token = parser.eat();
				if (token.type == TokenType::PUNCTUATOR_CLOSE_SQUARE_BRACKET) {
					break;
				}
				if (token.type == TokenType::PUNCTUATOR_COMMA) {
					if (parser.peek().type == TokenType::PUNCTUATOR_CLOSE_SQUARE_BRACKET) {
						parser.advance();
						break;
					}
				} else {
					throw Error{"Expected a comma or closing bracket.", token.source};
				}
			}
		}
		return source;
	}

	/**
	 * Read a single JSON value from the input.
	 *
	 * \param value reference to the output value to write the parsed result to.
	 *
	 * \return the location of the parsed value.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input stream.
	 *
	 * \note If any exception is thrown, the output value is left unmodified.
	 *
	 * \sa Parser::parseValue()
	 */
	SourceLocation readValue(Value& value) {
		const SourceLocation source = parser.peek().source;
		value = parser.parseValue();
		return source;
	}

	/**
	 * Read a single nullable JSON value from the input into any value that is
	 * default-constructible, move-assignable and dereferencable, and where the
	 * dereferenced value type is default-constructible, deserializable from
	 * JSON and can be move-assigned into the value.
	 *
	 * If a null value is read, the output is assigned a default-constructed
	 * value of its own type.
	 *
	 * \param value reference to the output value to write the result to.
	 *
	 * \return the location of the parsed value.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 * \throws any exception thrown by the default constructor of the given
	 *         value type or its dereferenced type.
	 * \throws any exception thrown by the Deserializer implementation of the
	 *         given value's dereferenced type.
	 * \throws any exception thrown by the assignment operator of the given
	 *         value type.
	 */
	template <typename T>
	SourceLocation readOptional(T& value) {
		const SourceLocation source = parser.peek().source;
		if (parser.peek().type == TokenType::IDENTIFIER_NULL) {
			parser.advance();
			value = T{};
		} else {
			std::remove_cvref_t<decltype(*value)> result{};
			deserialize(result);
			value = std::move(result);
		}
		return source;
	}

	/**
	 * Read a single JSON value from the input into any value of aggregate type
	 * whose fields are deserializable from JSON.
	 *
	 * \param value reference to the output value whose fields to write the
	 *        parsed results into.
	 *
	 * \return the location of the beginning of the parsed value.
	 *
	 * \throws json::Error on invalid input.
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the underlying input iterator.
	 * \throws any exception thrown by the Deserializer implementations of the
	 *         given field types.
	 *
	 * \warning If an exception is thrown, the output value may be left with
	 *          some successfully parsed fields, since they are
	 *          not reset automatically if a later operation fails.
	 */
	template <typename T>
	SourceLocation readAggregate(T& value) {
		const SourceLocation source = parser.peek().source;
		if constexpr (reflection::aggregate_size_v<T> == 1) {
			auto& [v] = value;
			deserialize(v);
		} else {
			if (const Token token = parser.eat(); token.type != TokenType::PUNCTUATOR_OPEN_SQUARE_BRACKET) {
				throw Error{"Expected an array.", token.source};
			}
			bool successor = false;
			reflection::forEach(reflection::fields(value), [&](auto& v) -> void {
				if (successor) {
					if (const Token token = parser.eat(); token.type != TokenType::PUNCTUATOR_COMMA) {
						throw Error{"Expected a comma.", token.source};
					}
				}
				successor = true;
				deserialize(v);
			});
			Token token = parser.eat();
			if (token.type == TokenType::PUNCTUATOR_COMMA) {
				token = parser.eat();
			}
			if (token.type != TokenType::PUNCTUATOR_CLOSE_SQUARE_BRACKET) {
				throw Error{"Missing end of array.", token.source};
			}
		}
		return source;
	}

	/**
	 * Read a JSON value from the input into any value that is deserializable
	 * from JSON using its corresponding implementation of Deserializer.
	 *
	 * \param value reference to the output value to write the parsed result to.
	 *
	 * \throws any exception thrown by the underlying input stream.
	 * \throws any exception thrown by the Deserializer implementation of T.
	 */
	template <typename T>
	void deserialize(T& value) {
		json::Deserializer<std::remove_cvref_t<T>>{}.deserialize(*this, value);
	}
};

/**
 * Write any JSON-serializable value to an output stream using its corresponding
 * implementation of Serializer.
 *
 * \param stream stream to write into.
 * \param value value to write.
 * \param options output options, see SerializationOptions.
 *
 * \throws any exception thrown by the underlying output stream.
 * \throws any exception thrown by the Serializer implementation of T.
 *
 * \sa Writer
 */
template <typename T>
inline void serialize(std::ostream& stream, const T& value, const SerializationOptions& options) {
	Writer{stream, options}.serialize(value);
}

/**
 * Read a JSON value from an input stream into any value that is deserializable
 * from JSON using its corresponding implementation of Deserializer.
 *
 * \param stream stream to read from.
 * \param value reference to the output value to write the parsed result to.
 * \param options input options, see DeserializationOptions.
 *
 * \throws any exception thrown by the underlying input stream.
 * \throws any exception thrown by the Deserializer implementation of T.
 *
 * \sa Reader
 */
template <typename T>
inline void deserialize(std::istream& stream, T& value, const DeserializationOptions& options) {
	Reader{stream, options}.deserialize(value);
}

namespace detail {

struct NoVisitor {};

template <typename Base, typename Callback>
struct VisitNull : Base {
	[[no_unique_address]] Callback callback;

	VisitNull(Callback callback)
		: callback(std::move(callback)) {}

	VisitNull(Base base, Callback callback)
		: Base(std::move(base))
		, callback(std::move(callback)) {}

	void visitNull(const SourceLocation& source, Null value) {
		callback(source, value);
	}

	template <typename NewBase>
	[[nodiscard]] auto operator|(NewBase other) && {
		return VisitNull<NewBase, Callback>{std::move(other), std::move(callback)};
	}
};

template <typename Base, typename Callback>
struct VisitBoolean : Base {
	[[no_unique_address]] Callback callback;

	VisitBoolean(Callback callback)
		: callback(std::move(callback)) {}

	VisitBoolean(Base base, Callback callback)
		: Base(std::move(base))
		, callback(std::move(callback)) {}

	void visitBoolean(const SourceLocation& source, Boolean value) {
		callback(source, value);
	}

	template <typename NewBase>
	[[nodiscard]] auto operator|(NewBase other) && {
		return VisitBoolean<NewBase, Callback>{std::move(other), std::move(callback)};
	}
};

template <typename Base, typename Callback>
struct VisitString : Base {
	[[no_unique_address]] Callback callback;

	VisitString(Callback callback)
		: callback(std::move(callback)) {}

	VisitString(Base base, Callback callback)
		: Base(std::move(base))
		, callback(std::move(callback)) {}

	void visitString(const SourceLocation& source, String&& value) {
		callback(source, std::move(value));
	}

	template <typename NewBase>
	[[nodiscard]] auto operator|(NewBase other) && {
		return VisitString<NewBase, Callback>{std::move(other), std::move(callback)};
	}
};

template <typename Base, typename Callback>
struct VisitNumber : Base {
	[[no_unique_address]] Callback callback;

	VisitNumber(Callback callback)
		: callback(std::move(callback)) {}

	VisitNumber(Base base, Callback callback)
		: Base(std::move(base))
		, callback(std::move(callback)) {}

	void visitNumber(const SourceLocation& source, Number value) {
		callback(source, value);
	}

	template <typename NewBase>
	[[nodiscard]] auto operator|(NewBase other) && {
		return VisitNumber<NewBase, Callback>{std::move(other), std::move(callback)};
	}
};

template <typename Base, typename Callback>
struct VisitObject : Base {
	[[no_unique_address]] Callback callback;

	VisitObject(Callback callback)
		: callback(std::move(callback)) {}

	VisitObject(Base base, Callback callback)
		: Base(std::move(base))
		, callback(std::move(callback)) {}

	void visitObject(const SourceLocation& source, auto& parser) {
		callback(source, parser);
	}

	template <typename NewBase>
	[[nodiscard]] auto operator|(NewBase other) && {
		return VisitObject<NewBase, Callback>{std::move(other), std::move(callback)};
	}
};

template <typename Base, typename Callback>
struct VisitArray : Base {
	[[no_unique_address]] Callback callback;

	VisitArray(Callback callback)
		: callback(std::move(callback)) {}

	VisitArray(Base base, Callback callback)
		: Base(std::move(base))
		, callback(std::move(callback)) {}

	void visitArray(const SourceLocation& source, auto& parser) {
		callback(source, parser);
	}

	template <typename NewBase>
	[[nodiscard]] auto operator|(NewBase other) && {
		return VisitArray<NewBase, Callback>{std::move(other), std::move(callback)};
	}
};

template <typename Base, typename Callback>
struct VisitProperty : Base {
	[[no_unique_address]] Callback callback;

	VisitProperty(Callback callback)
		: callback(std::move(callback)) {}

	VisitProperty(Base base, Callback callback)
		: Base(std::move(base))
		, callback(std::move(callback)) {}

	void visitProperty(const SourceLocation& source, String&& key, auto& parser) {
		callback(source, std::move(key), parser);
	}

	template <typename NewBase>
	[[nodiscard]] auto operator|(NewBase other) && {
		return VisitProperty<NewBase, Callback>{std::move(other), std::move(callback)};
	}
};

} // namespace detail

/**
 * Build a Parser::ValueVisitor that handles Null values with a given callback
 * function.
 *
 * \param callback function object that is callable with the same signature as
 *        ValueVisitor::visitNull() and has the same semantics.
 *
 * \return a visitor that uses the given callback for visiting values of type
 *         Null. This visitor can be combined with other related visitors using
 *         the pipe operator '|'.
 *
 * \throws any exception thrown by the move constructor of the callback type.
 *
 * \sa json::onBoolean()
 * \sa json::onString()
 * \sa json::onNumber()
 * \sa json::onObject()
 * \sa json::onArray()
 * \sa Parser::parseFile(Parser::ValueVisitor&)
 * \sa Parser::parseValue(Parser::ValueVisitor&)
 * \sa Parser::parseArray(Parser::ValueVisitor&)
 */
template <typename Callback>
[[nodiscard]] inline auto onNull(Callback callback) {
	return detail::VisitNull<detail::NoVisitor, Callback>{std::move(callback)};
}

/**
 * Build a Parser::ValueVisitor that handles Boolean values with a given
 * callback function.
 *
 * \param callback function object that is callable with the same signature as
 *        ValueVisitor::visitBoolean() and has the same semantics.
 *
 * \return a visitor that uses the given callback for visiting values of type
 *         Boolean. This visitor can be combined with other related visitors
 *         using the pipe operator '|'.
 *
 * \throws any exception thrown by the move constructor of the callback type.
 *
 * \sa json::onNull()
 * \sa json::onString()
 * \sa json::onNumber()
 * \sa json::onObject()
 * \sa json::onArray()
 * \sa Parser::parseFile(Parser::ValueVisitor&)
 * \sa Parser::parseValue(Parser::ValueVisitor&)
 * \sa Parser::parseArray(Parser::ValueVisitor&)
 */
template <typename Callback>
[[nodiscard]] inline auto onBoolean(Callback callback) {
	return detail::VisitBoolean<detail::NoVisitor, Callback>{std::move(callback)};
}

/**
 * Build a Parser::ValueVisitor that handles String values with a given
 * callback function.
 *
 * \param callback function object that is callable with the same signature as
 *        ValueVisitor::visitString() and has the same semantics.
 *
 * \return a visitor that uses the given callback for visiting values of type
 *         String. This visitor can be combined with other related visitors using the
 *         pipe operator '|'.
 *
 * \throws any exception thrown by the move constructor of the callback type.
 *
 * \sa json::onNull()
 * \sa json::onBoolean()
 * \sa json::onNumber()
 * \sa json::onObject()
 * \sa json::onArray()
 * \sa Parser::parseFile(Parser::ValueVisitor&)
 * \sa Parser::parseValue(Parser::ValueVisitor&)
 * \sa Parser::parseArray(Parser::ValueVisitor&)
 */
template <typename Callback>
[[nodiscard]] inline auto onString(Callback callback) {
	return detail::VisitString<detail::NoVisitor, Callback>{std::move(callback)};
}

/**
 * Build a Parser::ValueVisitor that handles Number values with a given
 * callback function.
 *
 * \param callback function object that is callable with the same signature as
 *        ValueVisitor::visitNumber() and has the same semantics.
 *
 * \return a visitor that uses the given callback for visiting values of type
 *         Number. This visitor can be combined with other related visitors
 *         using the pipe operator '|'.
 *
 * \throws any exception thrown by the move constructor of the callback type.
 *
 * \sa json::onNull()
 * \sa json::onBoolean()
 * \sa json::onString()
 * \sa json::onObject()
 * \sa json::onArray()
 * \sa Parser::parseFile(Parser::ValueVisitor&)
 * \sa Parser::parseValue(Parser::ValueVisitor&)
 * \sa Parser::parseArray(Parser::ValueVisitor&)
 */
template <typename Callback>
[[nodiscard]] inline auto onNumber(Callback callback) {
	return detail::VisitNumber<detail::NoVisitor, Callback>{std::move(callback)};
}

/**
 * Build a Parser::ValueVisitor that handles objects with a given callback
 * function.
 *
 * \param callback function object that is callable with the same signature as
 *        ValueVisitor::visitObject() and has the same semantics.
 *
 * \return a visitor that uses the given callback for visiting objects. This
 *         visitor can be combined with other related visitors using the pipe
 *         operator '|'.
 *
 * \throws any exception thrown by the move constructor of the callback type.
 *
 * \sa json::onNull()
 * \sa json::onBoolean()
 * \sa json::onString()
 * \sa json::onNumber()
 * \sa json::onArray()
 * \sa Parser::parseFile(Parser::ValueVisitor&)
 * \sa Parser::parseValue(Parser::ValueVisitor&)
 * \sa Parser::parseArray(Parser::ValueVisitor&)
 */
template <typename Callback>
[[nodiscard]] inline auto onObject(Callback callback) {
	return detail::VisitObject<detail::NoVisitor, Callback>{std::move(callback)};
}

/**
 * Build a Parser::ValueVisitor that handles arrays with a given callback
 * function.
 *
 * \param callback function object that is callable with the same signature as
 *        ValueVisitor::visitArray() and has the same semantics.
 *
 * \return a visitor that uses the given callback for visiting arrays. This
 *         visitor can be combined with other related visitors using the pipe
 *         operator '|'.
 *
 * \throws any exception thrown by the move constructor of the callback type.
 *
 * \sa json::onNull()
 * \sa json::onBoolean()
 * \sa json::onString()
 * \sa json::onNumber()
 * \sa json::onObject()
 * \sa Parser::parseFile(Parser::ValueVisitor&)
 * \sa Parser::parseValue(Parser::ValueVisitor&)
 * \sa Parser::parseArray(Parser::ValueVisitor&)
 */
template <typename Callback>
[[nodiscard]] inline auto onArray(Callback callback) {
	return detail::VisitArray<detail::NoVisitor, Callback>{std::move(callback)};
}

/**
 * Build a Parser::PropertyVisitor that handles object properties with a given
 * callback function.
 *
 * \param callback function object that is callable with the same signature as
 *        PropertyVisitor::visitProperty() and has the same semantics.
 *
 * \return a visitor that uses the given callback for visiting arrays. This
 *         visitor can be combined with other related visitors using the pipe
 *         operator '|'.
 *
 * \throws any exception thrown by the move constructor of the callback type.
 *
 * \sa Parser::parseObject(Parser::PropertyVisitor&)
 */
template <typename Callback>
[[nodiscard]] inline auto onProperty(Callback callback) {
	return detail::VisitProperty<detail::NoVisitor, Callback>{std::move(callback)};
}

namespace detail {

template <typename T>
concept nullable =              //
	!std::is_arithmetic_v<T> && //
	requires(const T& value) {
		static_cast<bool>(value);
		static_cast<bool>(!value);
		T{};
	};

template <typename T>
concept serializable_as_string =                                //
	std::is_same_v<T, String> ||                                //
	requires(const T& value) { std::string_view{value}; } ||    //
	requires(const T& value) { std::u8string_view{value}; } ||  //
	requires(const T& value) { std::u16string_view{value}; } || //
	requires(const T& value) { std::u32string_view{value}; } || //
	requires(const T& value) { std::wstring_view{value}; };

template <typename T>
concept deserializable_as_string =                      //
	std::is_same_v<T, String> ||                        //
	requires(T& value) { value = std::string{}; } ||    //
	requires(T& value) { value = std::u8string{}; } ||  //
	requires(T& value) { value = std::u16string{}; } || //
	requires(T& value) { value = std::u32string{}; } || //
	requires(T& value) { value = std::wstring{}; };

template <typename T>
concept serializable_as_object = //
	std::is_same_v<T, Object> || //
	requires(Writer& writer, const T& value) {
		writer.writeString(std::begin(value)->first);
		writer.serialize(std::begin(value)->second);
	};

template <typename T>
concept deserializable_as_object = //
	std::is_same_v<T, Object> ||   //
	requires(Reader& reader, T& value) {
		value.clear();
		std::remove_cvref_t<decltype(std::begin(value)->first)>{};
		std::remove_cvref_t<decltype(std::begin(value)->second)>{};
		reader.readString(std::begin(value)->first);
		reader.deserialize(std::begin(value)->second);
		value.emplace(std::remove_cvref_t<decltype(std::begin(value)->first)>{}, std::remove_cvref_t<decltype(std::begin(value)->second)>{});
	};

template <typename T>
concept serializable_as_array = //
	std::is_same_v<T, Array> || //
	requires(Writer& writer, const T& value) { writer.serialize(*std::begin(value)); };

template <typename T>
concept deserializable_as_array = //
	std::is_same_v<T, Array> ||   //
	requires(Reader& reader, T& value) {
		value.clear();
		std::remove_cvref_t<decltype(*std::begin(value))>{};
		reader.deserialize(*std::begin(value));
		value.push_back(std::remove_cvref_t<decltype(*std::begin(value))>{});
	};

template <typename T>
concept serializable_as_optional = //
	!std::is_pointer_v<T> &&       //
	requires(Writer& writer, const T& value) {
		static_cast<bool>(value);
		writer.serialize(*value);
	};

template <typename T>
concept deserializable_as_optional = //
	!std::is_pointer_v<T> &&         //
	requires(Reader& reader, T& value, std::remove_cvref_t<decltype(*value)> result) {
		value = T{};
		std::remove_cvref_t<decltype(*value)>{};
		reader.deserialize(result);
		value = std::move(result);
	};

template <typename T>
inline constexpr bool always_false_v = false;

template <typename T, typename ObjectPropertyFilter, typename ArrayItemFilter>
inline std::size_t getRecursiveSize(const T& value, ObjectPropertyFilter objectPropertyFilter, ArrayItemFilter arrayItemFilter) {
	if constexpr (std::is_same_v<T, Value>) {
		return match(value)([&](const auto& v) -> std::size_t { return getRecursiveSize(v, objectPropertyFilter, arrayItemFilter); });
	} else if constexpr (serializable_as_string<T>) {
		return 1;
	} else if constexpr (serializable_as_object<T>) {
		if constexpr (nullable<T>) {
			if (!value) {
				return 1;
			}
		}
		return std::accumulate(std::begin(value), std::end(value), std::size_t{1}, [&](std::size_t count, const auto& kv) -> std::size_t {
			if (objectPropertyFilter(kv)) {
				count += getRecursiveSize(kv.second, {}, {});
			}
			return count;
		});
	} else if constexpr (serializable_as_array<T>) {
		if constexpr (nullable<T>) {
			if (!value) {
				return 1;
			}
		}
		return std::accumulate(std::begin(value), std::end(value), std::size_t{1}, [&](std::size_t count, const auto& v) -> std::size_t {
			if (arrayItemFilter(v)) {
				count += getRecursiveSize(v, {}, {});
			}
			return count;
		});
	} else if constexpr (serializable_as_optional<T>) {
		return 1;
	} else if constexpr (std::is_aggregate_v<T>) {
		if constexpr (nullable<T>) {
			if (!value) {
				return 1;
			}
		}
		std::size_t result = 1;
		reflection::forEach(reflection::fields(value), [&](const auto& v) -> void { result += getRecursiveSize(v, {}, {}); });
		return result;
	} else {
		return 1;
	}
}

} // namespace detail

template <typename T>
struct Serializer {
	void serialize(Writer& writer, const T& value) {
		if constexpr (detail::nullable<T>) {
			if (!value) {
				writer.writeNull();
				return;
			}
		}
		if constexpr (detail::serializable_as_string<T>) {
			writer.writeString(value);
		} else if constexpr (detail::serializable_as_object<T>) {
			writer.writeObject(value);
		} else if constexpr (detail::serializable_as_array<T>) {
			writer.writeArray(value);
		} else if constexpr (detail::serializable_as_optional<T>) {
			writer.writeOptional(value);
		} else if constexpr (std::is_aggregate_v<T>) {
			writer.writeAggregate(value);
		} else {
			static_assert(detail::always_false_v<T>, "JSON serialization is not implemented for the given type.");
		}
	}
};

template <typename T>
struct Deserializer {
	void deserialize(Reader& reader, T& value) {
		if constexpr (detail::serializable_as_string<T>) {
			reader.readString(value);
		} else if constexpr (detail::serializable_as_object<T>) {
			reader.readObject(value);
		} else if constexpr (detail::serializable_as_array<T>) {
			reader.readArray(value);
		} else if constexpr (detail::serializable_as_optional<T>) {
			reader.readOptional(value);
		} else if constexpr (std::is_aggregate_v<T>) {
			reader.readAggregate(value);
		} else {
			static_assert(detail::always_false_v<T>, "JSON deserialization is not implemented for the given type.");
		}
	}
};

/// \cond
template <>
struct Serializer<Null> {
	void serialize(Writer& writer, Null) {
		writer.writeNull();
	}
};

template <>
struct Serializer<std::nullptr_t> {
	void serialize(Writer& writer, std::nullptr_t) {
		writer.writeNull();
	}
};

template <>
struct Serializer<Boolean> {
	void serialize(Writer& writer, Boolean value) {
		writer.writeBoolean(value);
	}
};

template <detail::number Num>
struct Serializer<Num> {
	void serialize(Writer& writer, Num value) {
		writer.writeNumber(static_cast<Number>(value));
	}
};

template <>
struct Serializer<char> {
	void serialize(Writer& writer, char value) {
		writer.writeString(std::string_view{&value, 1});
	}
};

template <>
struct Serializer<char8_t> {
	void serialize(Writer& writer, char8_t value) {
		writer.writeString(std::u8string_view{&value, 1});
	}
};

template <>
struct Serializer<char16_t> {
	void serialize(Writer& writer, char16_t value) {
		writer.writeString(std::u16string_view{&value, 1});
	}
};

template <>
struct Serializer<char32_t> {
	void serialize(Writer& writer, char32_t value) {
		writer.writeString(std::u32string_view{&value, 1});
	}
};

template <>
struct Serializer<wchar_t> {
	void serialize(Writer& writer, wchar_t value) {
		writer.writeString(std::wstring_view{&value, 1});
	}
};

template <>
struct Serializer<Value> {
	void serialize(Writer& writer, const Value& value) {
		match(value)([&](const auto& v) -> void { writer.serialize(v); });
	}
};
/// \endcond

/// \cond
template <>
struct Deserializer<Null> {
	void deserialize(Reader& reader, Null&) {
		reader.readNull();
	}
};

template <>
struct Deserializer<std::nullptr_t> {
	void deserialize(Reader& reader, std::nullptr_t&) {
		reader.readNull();
	}
};

template <>
struct Deserializer<Boolean> {
	void deserialize(Reader& reader, Boolean& value) {
		reader.readBoolean(value);
	}
};

template <detail::number Num>
struct Deserializer<Num> {
	void deserialize(Reader& reader, Num& value) {
		reader.readNumber(value);
	}
};

template <>
struct Deserializer<char> {
	void deserialize(Reader& reader, char& value) {
		String string{};
		const SourceLocation source = reader.readString(string);
		if (string.size() != 1) {
			throw Error{"Expected only a single character.", source};
		}
		value = string.front();
	}
};

template <>
struct Deserializer<char8_t> {
	void deserialize(Reader& reader, char8_t& value) {
		String string{};
		const SourceLocation source = reader.readString(string);
		if (string.size() != sizeof(char8_t)) {
			throw Error{"Expected only a single UTF-8 code unit.", source};
		}
		std::memcpy(&value, string.data(), sizeof(char8_t));
	}
};

template <>
struct Deserializer<char16_t> {
	void deserialize(Reader& reader, char16_t& value) {
		String string{};
		const SourceLocation source = reader.readString(string);
		if (string.size() != sizeof(char16_t)) {
			throw Error{"Expected only a single UTF-16 code unit.", source};
		}
		std::memcpy(&value, string.data(), sizeof(char16_t));
	}
};

template <>
struct Deserializer<char32_t> {
	void deserialize(Reader& reader, char32_t& value) {
		String string{};
		const SourceLocation source = reader.readString(string);
		if (string.size() != sizeof(char32_t)) {
			throw Error{"Expected only a single UTF-32 code unit.", source};
		}
		std::memcpy(&value, string.data(), sizeof(char32_t));
	}
};

template <>
struct Deserializer<wchar_t> {
	void deserialize(Reader& reader, wchar_t& value) {
		String string{};
		const SourceLocation source = reader.readString(string);
		if (string.size() != sizeof(wchar_t)) {
			throw Error{"Expected only a single wide character.", source};
		}
		std::memcpy(&value, string.data(), sizeof(wchar_t));
	}
};

template <>
struct Deserializer<Value> {
	void deserialize(Reader& reader, Value& value) {
		reader.readValue(value);
	}
};
/// \endcond

inline Object::Object() noexcept = default;

inline Object::~Object() = default;

inline Object::Object(const Object& other) = default;

inline Object::Object(Object&& other) noexcept = default;

inline Object& Object::operator=(const Object& other) = default;

inline Object& Object::operator=(Object&& other) noexcept = default;

template <typename InputIt>
inline Object::Object(InputIt first, InputIt last)
	: membersSortedByName(first, last) {
	std::sort(membersSortedByName.begin(), membersSortedByName.end(), Compare{});
}

inline Object::Object(std::initializer_list<value_type> ilist)
	: Object(ilist.begin(), ilist.end()) {}

inline Object& Object::operator=(std::initializer_list<value_type> ilist) {
	membersSortedByName = ilist;
	std::sort(membersSortedByName.begin(), membersSortedByName.end(), Compare{});
	return *this;
}

inline Value& Object::at(std::string_view name) {
	if (const auto it = find(name); it != end()) {
		return it->second;
	}
	throw std::out_of_range{"JSON object does not contain a member with the given name."};
}

inline const Value& Object::at(std::string_view name) const {
	if (const auto it = find(name); it != end()) {
		return it->second;
	}
	throw std::out_of_range{"JSON object does not contain a member with the given name."};
}

inline Value& Object::operator[](const String& k) {
	return try_emplace(k).first->second;
}

inline Value& Object::operator[](String&& k) {
	return try_emplace(std::move(k)).first->second;
}

inline Object::iterator Object::begin() noexcept {
	return membersSortedByName.begin();
}

inline Object::const_iterator Object::begin() const noexcept {
	return membersSortedByName.begin();
}

inline Object::const_iterator Object::cbegin() const noexcept {
	return membersSortedByName.cbegin();
}

inline Object::iterator Object::end() noexcept {
	return membersSortedByName.end();
}

inline Object::const_iterator Object::end() const noexcept {
	return membersSortedByName.end();
}

inline Object::const_iterator Object::cend() const noexcept {
	return membersSortedByName.cend();
}

inline Object::reverse_iterator Object::rbegin() noexcept {
	return membersSortedByName.rbegin();
}

inline Object::const_reverse_iterator Object::rbegin() const noexcept {
	return membersSortedByName.rbegin();
}

inline Object::const_reverse_iterator Object::crbegin() const noexcept {
	return membersSortedByName.crbegin();
}

inline Object::reverse_iterator Object::rend() noexcept {
	return membersSortedByName.rend();
}

inline Object::const_reverse_iterator Object::rend() const noexcept {
	return membersSortedByName.rend();
}

inline Object::const_reverse_iterator Object::crend() const noexcept {
	return membersSortedByName.crend();
}

inline bool Object::empty() const noexcept {
	return membersSortedByName.empty();
}

inline Object::size_type Object::size() const noexcept {
	return membersSortedByName.size();
}

inline Object::size_type Object::max_size() const noexcept {
	return membersSortedByName.max_size();
}

inline void Object::clear() noexcept {
	membersSortedByName.clear();
}

template <typename P>
inline std::pair<Object::iterator, bool> Object::insert(P&& value) {
	return emplace(std::forward<P>(value));
}

template <typename P>
inline Object::iterator Object::insert(const_iterator pos, P&& value) {
	return emplace_hint(pos, std::forward<P>(value));
}

template <typename InputIt>
inline void Object::insert(InputIt first, InputIt last) {
	while (first != last) {
		insert(*first++);
	}
}

inline void Object::insert(std::initializer_list<Object::value_type> ilist) {
	insert(ilist.begin(), ilist.end());
}

template <typename... Args>
inline std::pair<Object::iterator, bool> Object::emplace(Args&&... args) {
	value_type value{std::forward<Args>(args)...};
	const auto [first, last] = equal_range(value.first);
	if (first != last) {
		return {first, false};
	}
	const auto it = membersSortedByName.insert(last, std::move(value));
	return {it, true};
}

template <typename... Args>
inline Object::iterator Object::emplace_hint(const_iterator, Args&&... args) {
	return emplace(std::forward<Args>(args)...);
}

template <typename... Args>
inline std::pair<Object::iterator, bool> Object::try_emplace(const String& k, Args&&... args) {
	const auto [first, last] = equal_range(k);
	if (first != last) {
		return {first, false};
	}
	const auto it = membersSortedByName.emplace(last, std::piecewise_construct, std::forward_as_tuple(k), std::forward_as_tuple(std::forward<Args>(args)...));
	return {it, true};
}

template <typename... Args>
inline std::pair<Object::iterator, bool> Object::try_emplace(String&& k, Args&&... args) {
	const auto [first, last] = equal_range(k);
	if (first != last) {
		return {first, false};
	}
	const auto it = membersSortedByName.emplace(last, std::piecewise_construct, std::forward_as_tuple(std::move(k)), std::forward_as_tuple(std::forward<Args>(args)...));
	return {it, true};
}

template <typename... Args>
inline Object::iterator Object::try_emplace(const_iterator, const String& k, Args&&... args) {
	return try_emplace(k, std::forward<Args>(args)...);
}

template <typename... Args>
inline Object::iterator Object::try_emplace(const_iterator, String&& k, Args&&... args) {
	return try_emplace(std::move(k), std::forward<Args>(args)...);
}

inline Object::iterator Object::erase(const_iterator pos) {
	return membersSortedByName.erase(pos);
}

inline Object::size_type Object::erase(std::string_view name) {
	const auto [first, last] = equal_range(name);
	const size_type count = static_cast<size_type>(last - first);
	membersSortedByName.erase(first, last);
	return count;
}

inline void Object::swap(Object& other) noexcept {
	membersSortedByName.swap(other.membersSortedByName);
}

inline void swap(Object& a, Object& b) noexcept {
	a.swap(b);
}

inline Object::size_type Object::count(std::string_view name) const noexcept {
	const auto [first, last] = equal_range(name);
	return static_cast<size_type>(last - first);
}

inline bool Object::contains(std::string_view name) const noexcept {
	return count(name) > 0;
}

inline Object::iterator Object::find(std::string_view name) noexcept {
	if (const auto [first, last] = equal_range(name); first != last) {
		return first;
	}
	return end();
}

inline Object::const_iterator Object::find(std::string_view name) const noexcept {
	if (const auto [first, last] = equal_range(name); first != last) {
		return first;
	}
	return end();
}

inline std::pair<Object::iterator, Object::iterator> Object::equal_range(std::string_view name) noexcept {
	return std::equal_range(membersSortedByName.begin(), membersSortedByName.end(), name, Compare{});
}

inline std::pair<Object::const_iterator, Object::const_iterator> Object::equal_range(std::string_view name) const noexcept {
	return std::equal_range(membersSortedByName.begin(), membersSortedByName.end(), name, Compare{});
}

inline Object::iterator Object::lower_bound(std::string_view name) noexcept {
	return std::lower_bound(membersSortedByName.begin(), membersSortedByName.end(), name, Compare{});
}

inline Object::const_iterator Object::lower_bound(std::string_view name) const noexcept {
	return std::lower_bound(membersSortedByName.begin(), membersSortedByName.end(), name, Compare{});
}

inline Object::iterator Object::upper_bound(std::string_view name) noexcept {
	return std::upper_bound(membersSortedByName.begin(), membersSortedByName.end(), name, Compare{});
}

inline Object::const_iterator Object::upper_bound(std::string_view name) const noexcept {
	return std::upper_bound(membersSortedByName.begin(), membersSortedByName.end(), name, Compare{});
}

inline bool Object::operator==(const Object& other) const noexcept {
	return membersSortedByName == other.membersSortedByName;
}

inline std::partial_ordering Object::operator<=>(const Object& other) const noexcept {
	return std::compare_partial_order_fallback(membersSortedByName, other.membersSortedByName);
}

template <typename Predicate>
inline Object::size_type erase_if(Object& container, Predicate predicate) {
	return std::erase_if(container.membersSortedByName, predicate);
}

inline bool Object::Compare::operator()(const value_type& a, const value_type& b) const noexcept {
	return a.first < b.first;
}

inline bool Object::Compare::operator()(const value_type& a, std::string_view b) const noexcept {
	return a.first < b;
}

inline bool Object::Compare::operator()(std::string_view a, const value_type& b) const noexcept {
	return a < b.first;
}

inline bool Object::Compare::operator()(std::string_view a, std::string_view b) const noexcept {
	return a < b;
}

inline Array::Array() noexcept = default;

inline Array::~Array() = default;

inline Array::Array(const Array& other) = default;

inline Array::Array(Array&& other) noexcept = default;

inline Array& Array::operator=(const Array& other) = default;

inline Array& Array::operator=(Array&& other) noexcept = default;

template <typename InputIt>
inline Array::Array(InputIt first, InputIt last)
	: values(first, last) {}

inline Array::Array(size_type count, const Value& value)
	: values(count, value) {}

inline Array::Array(std::initializer_list<value_type> ilist)
	: values(ilist) {}

inline Array& Array::operator=(std::initializer_list<value_type> ilist) {
	values = ilist;
	return *this;
}

inline void Array::swap(Array& other) noexcept {
	values.swap(other.values);
}

inline void swap(Array& a, Array& b) noexcept {
	a.swap(b);
}

inline Array::pointer Array::data() noexcept {
	return values.data();
}

inline Array::const_pointer Array::data() const noexcept {
	return values.data();
}

inline Array::size_type Array::size() const noexcept {
	return values.size();
}

inline Array::size_type Array::max_size() const noexcept {
	return values.max_size();
}

inline Array::size_type Array::capacity() const noexcept {
	return values.capacity();
}

inline bool Array::empty() const noexcept {
	return values.empty();
}

inline Array::iterator Array::begin() noexcept {
	return values.begin();
}

inline Array::const_iterator Array::begin() const noexcept {
	return values.begin();
}

inline Array::const_iterator Array::cbegin() const noexcept {
	return values.cbegin();
}

inline Array::iterator Array::end() noexcept {
	return values.end();
}

inline Array::const_iterator Array::end() const noexcept {
	return values.end();
}

inline Array::const_iterator Array::cend() const noexcept {
	return values.cend();
}

inline Array::reverse_iterator Array::rbegin() noexcept {
	return values.rbegin();
}

inline Array::const_reverse_iterator Array::rbegin() const noexcept {
	return values.rbegin();
}

inline Array::const_reverse_iterator Array::crbegin() const noexcept {
	return values.crbegin();
}

inline Array::reverse_iterator Array::rend() noexcept {
	return values.rend();
}

inline Array::const_reverse_iterator Array::rend() const noexcept {
	return values.rend();
}

inline Array::const_reverse_iterator Array::crend() const noexcept {
	return values.crend();
}

inline Array::reference Array::front() {
	return values.front();
}

inline Array::const_reference Array::front() const {
	return values.front();
}

inline Array::reference Array::back() {
	return values.back();
}

inline Array::const_reference Array::back() const {
	return values.back();
}

inline Array::reference Array::at(size_type pos) {
	return values.at(pos);
}

inline Array::const_reference Array::at(size_type pos) const {
	return values.at(pos);
}

inline Array::reference Array::operator[](size_type pos) {
	return values[pos];
}

inline Array::const_reference Array::operator[](size_type pos) const {
	return values[pos];
}

inline bool Array::operator==(const Array& other) const {
	return values == other.values;
}

inline std::partial_ordering Array::operator<=>(const Array& other) const noexcept {
	return std::compare_partial_order_fallback(values, other.values);
}

template <typename U>
inline Array::size_type erase(Array& container, const U& value) {
	return std::erase(container.values, value);
}

template <typename Predicate>
inline Array::size_type erase_if(Array& container, Predicate predicate) {
	return std::erase_if(container.values, predicate);
}

inline void Array::clear() noexcept {
	values.clear();
}

inline void Array::reserve(size_type newCap) {
	values.reserve(newCap);
}

inline void Array::shrink_to_fit() {
	values.shrink_to_fit();
}

inline Array::iterator Array::insert(const_iterator pos, const Value& value) {
	return values.insert(pos, value);
}

inline Array::iterator Array::insert(const_iterator pos, Value&& value) {
	return values.insert(pos, std::move(value));
}

inline Array::iterator Array::insert(const_iterator pos, size_type count, const Value& value) {
	return values.insert(pos, count, value);
}

template <typename InputIt>
inline Array::iterator Array::insert(const_iterator pos, InputIt first, InputIt last) {
	return values.insert(pos, first, last);
}

inline Array::iterator Array::insert(const_iterator pos, std::initializer_list<value_type> ilist) {
	return values.insert(pos, ilist);
}

template <typename... Args>
inline Array::iterator Array::emplace(const_iterator pos, Args&&... args) {
	return values.emplace(pos, std::forward<Args>(args)...);
}

inline Array::iterator Array::erase(const_iterator pos) {
	return values.erase(pos);
}

inline Array::iterator Array::erase(const_iterator first, const_iterator last) {
	return values.erase(first, last);
}

inline void Array::push_back(const Value& value) {
	values.push_back(value);
}

inline void Array::push_back(Value&& value) {
	values.push_back(std::move(value));
}

template <typename... Args>
inline Array::reference Array::emplace_back(Args&&... args) {
	return values.emplace_back(std::forward<Args>(args)...);
}

inline void Array::pop_back() {
	values.pop_back();
}

inline void Array::resize(size_type count) {
	values.resize(count);
}

inline void Array::resize(size_type count, const Value& value) {
	values.resize(count, value);
}

inline Value Value::parse(std::u8string_view jsonString) {
	unicode::UTF8View codePoints{jsonString};
	return Parser<const char8_t*>{{codePoints.begin(), codePoints.end(), {.lineNumber = 1, .columnNumber = 1}}}.parseFile();
}

inline Value Value::parse(std::string_view jsonString) {
	static_assert(sizeof(char) == sizeof(char8_t));
	static_assert(alignof(char) == alignof(char8_t));
	return parse(std::u8string_view{reinterpret_cast<const char8_t*>(jsonString.data()), jsonString.size()});
}

inline std::string Value::toString(const SerializationOptions& options) const {
	std::ostringstream stream{};
	json::serialize(stream, *this, options);
	return std::move(stream).str();
}

} // namespace donut::json

#endif
