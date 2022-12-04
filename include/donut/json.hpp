#ifndef DONUT_JSON_HPP
#define DONUT_JSON_HPP

#include <donut/Variant.hpp>
#include <donut/reflection.hpp>
#include <donut/unicode.hpp>

#include <algorithm>
#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <ios>
#include <istream>
#include <iterator>
#include <limits>
#include <numeric>
#include <ostream>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace donut {
namespace json {

namespace detail {

template <typename T>
concept number = std::is_arithmetic_v<T> && !std::is_same_v<T, bool> && !std::is_same_v<T, char> && !std::is_same_v<T, char8_t> && !std::is_same_v<T, char16_t> &&
	!std::is_same_v<T, char32_t> && !std::is_same_v<T, wchar_t>;

} // namespace detail

struct SourceLocation {
	std::size_t lineNumber;
	std::size_t columnNumber;
};

struct Error : std::runtime_error {
	SourceLocation source;

	Error(const std::string& message, const SourceLocation& source)
		: std::runtime_error(message)
		, source(source) {}

	Error(const char* message, const SourceLocation& source)
		: std::runtime_error(message)
		, source(source) {}
};

class Value;

using Null = Monostate;

using Boolean = bool;

using String = std::string;

using Number = double;

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
	std::pair<iterator, bool> insert(P&& value) requires(std::is_constructible_v<value_type, P&&>);

	template <typename P>
	iterator insert(const_iterator pos, P&& value) requires(std::is_constructible_v<value_type, P&&>);

	template <typename InputIt>
	void insert(InputIt first, InputIt last);
	void insert(std::initializer_list<value_type> ilist);

	template <typename... Args>
	std::pair<iterator, bool> emplace(Args&&... args) requires(std::is_constructible_v<value_type, Args...>);

	template <typename... Args>
	iterator emplace_hint(const_iterator hint, Args&&... args) requires(std::is_constructible_v<value_type, Args...>);

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

	template <typename Pred>
	friend size_type erase_if(Object& c, Pred pred);

private:
	struct Compare {
		[[nodiscard]] bool operator()(const value_type& a, const value_type& b) const noexcept;
		[[nodiscard]] bool operator()(const value_type& a, std::string_view b) const noexcept;
		[[nodiscard]] bool operator()(std::string_view a, const value_type& b) const noexcept;
		[[nodiscard]] bool operator()(std::string_view a, std::string_view b) const noexcept;
	};

	std::vector<value_type> membersSortedByName;
};

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
	friend size_type erase(Array& c, const U& value);

	template <typename Pred>
	friend size_type erase_if(Array& c, Pred pred);

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

class Value : public Variant<Null, Boolean, String, Number, Object, Array> {
public:
	[[nodiscard]] static Value parse(std::u8string_view jsonString);
	[[nodiscard]] static Value parse(std::string_view jsonString);

	Value() noexcept = default;

	Value(Null value) noexcept
		: Variant(value) {}

	Value(std::nullptr_t) noexcept
		: Variant(Null{}) {}

	Value(Boolean value) noexcept
		: Variant(value) {}

	Value(const String& value) noexcept
		: Variant(value) {}

	Value(String&& value) noexcept
		: Variant(std::move(value)) {}

	Value(const char* value) noexcept
		: Variant(String{value}) {}

	Value(std::string_view value) noexcept
		: Variant(String{value}) {}

	Value(const char8_t* value) noexcept
		: Variant(String{reinterpret_cast<const char*>(value)}) {
		static_assert(sizeof(char) == sizeof(char8_t));
		static_assert(alignof(char) == alignof(char8_t));
	}

	Value(std::u8string_view value) noexcept
		: Variant(String{value.begin(), value.end()}) {}

	Value(detail::number auto value) noexcept
		: Variant(static_cast<Number>(value)) {}

	Value(const Object& value) noexcept
		: Variant(value) {}

	Value(Object&& value) noexcept
		: Variant(std::move(value)) {}

	Value(const Array& value) noexcept
		: Variant(value) {}

	Value(Array&& value) noexcept
		: Variant(std::move(value)) {}

	[[nodiscard]] std::string toString() const;

	[[nodiscard]] std::partial_ordering operator<=>(const Value& other) const {
		return static_cast<const Variant&>(*this) <=> static_cast<const Variant&>(other);
	}
};

namespace detail {

enum class TokenType : std::uint8_t {
	END_OF_FILE,
	IDENTIFIER_NULL,
	IDENTIFIER_FALSE,
	IDENTIFIER_TRUE,
	IDENTIFIER_NAME,
	PUNCTUATOR_COMMA,
	PUNCTUATOR_COLON,
	PUNCTUATOR_OPEN_SQUARE_BRACKET,
	PUNCTUATOR_CLOSE_SQUARE_BRACKET,
	PUNCTUATOR_OPEN_CURLY_BRACE,
	PUNCTUATOR_CLOSE_CURLY_BRACE,
	STRING,
	NUMBER_BINARY,
	NUMBER_OCTAL,
	NUMBER_DECIMAL,
	NUMBER_HEXADECIMAL,
	NUMBER_POSITIVE_INFINITY,
	NUMBER_NEGATIVE_INFINITY,
	NUMBER_POSITIVE_NAN,
	NUMBER_NEGATIVE_NAN,
};

template <typename It>
struct Token {
	String string;
	SourceLocation source;
	TokenType type;
};

template <typename It>
class Lexer {
public:
	[[nodiscard]] constexpr bool isWhitespaceCharacter(char32_t codePoint) noexcept {
		return codePoint == '\t' || codePoint == '\n' || codePoint == '\v' || codePoint == '\f' || codePoint == '\r' || codePoint == ' ' || codePoint == 0x00A0 ||
			codePoint == 0x1680 || (codePoint >= 0x2000 && codePoint <= 0x200A) || codePoint == 0x2028 || codePoint == 0x2029 || codePoint == 0x202F || codePoint == 0x205F ||
			codePoint == 0x3000 || codePoint == 0xFEFF;
	}

	[[nodiscard]] constexpr bool isPunctuationCharacter(char32_t codePoint) noexcept {
		return codePoint == ',' || codePoint == ':' || codePoint == '[' || codePoint == ']' || codePoint == '{' || codePoint == '}';
	}

	[[nodiscard]] constexpr bool isLineTerminatorCharacter(char32_t codePoint) noexcept {
		return codePoint == '\n' || codePoint == '\r' || codePoint == 0x2028 || codePoint == 0x2029;
	}

	Lexer(unicode::UTF8Iterator<It> it, unicode::UTF8Sentinel end, const SourceLocation& source);

	Token<It> scan();

private:
	void skipWhitespace();

	void skipLineTerminatorSequence();

	void advance();

	[[nodiscard]] bool hasReachedEnd() const noexcept;

	[[nodiscard]] char32_t peek() const;
	[[nodiscard]] std::optional<char32_t> lookahead() const;

	[[nodiscard]] Token<It> scanPunctuator();
	[[nodiscard]] Token<It> scanString();
	[[nodiscard]] Token<It> scanNumber();
	[[nodiscard]] Token<It> scanIdentifier();

	void scanNumericEscapeSequence(String& output, std::size_t minDigitCount, std::size_t maxDigitCount, int radix, bool (*isDigit)(char32_t) noexcept);

	mutable unicode::UTF8Iterator<It> it;
	unicode::UTF8Sentinel end;
	SourceLocation source;
	mutable std::optional<char32_t> currentCodePoint{};
};

template <typename It>
class Parser {
public:
	explicit Parser(Lexer<It> lexer);

	Value parseFile();

	std::pair<Null, SourceLocation> parseNull();
	std::pair<Boolean, SourceLocation> parseBoolean();
	std::pair<String, SourceLocation> parseString();
	std::pair<Number, SourceLocation> parseNumber();
	std::pair<Object, SourceLocation> parseObject();
	std::pair<Array, SourceLocation> parseArray();
	std::pair<Value, SourceLocation> parseValue();

	void advance();

	[[nodiscard]] const Token<It>& peek() const noexcept;

	[[nodiscard]] Token<It> eat();

private:
	[[nodiscard]] static Number parseNumberContents(Token<It> token, int radix);

	[[nodiscard]] Object parseObjectContents();
	[[nodiscard]] Array parseArrayContents();

	mutable Lexer<It> lexer;
	mutable std::optional<Token<It>> currentToken{};
};

template <typename T>
[[nodiscard]] std::size_t getRecursiveSize(const T& value);

} // namespace detail

template <typename T>
struct Serializer;

template <typename T>
struct Deserializer;

struct SerializationOptions {
	std::size_t indentation = 0;
	std::size_t relativeIndentation = 4;
	char indentationCharacter = ' ';
	bool prettyPrint = true;
};

struct DeserializationOptions {};

template <typename T>
void serialize(std::ostream& stream, const T& value, const SerializationOptions& options = {});

template <typename T>
void deserialize(std::istream& stream, T& value, const DeserializationOptions& options = {});

inline std::ostream& operator<<(std::ostream& stream, const Value& value) {
	json::serialize(stream, value);
	return stream;
}

inline std::istream& operator>>(std::istream& stream, Value& value) {
	try {
		json::deserialize(stream, value);
	} catch (const Error&) {
		stream.setstate(std::istream::failbit);
	}
	return stream;
}

struct SerializationState {
private:
	std::ostream& stream;

public:
	SerializationOptions options;

	void write(char byte) {
		stream << byte;
	}

	void write(std::string_view bytes) {
		stream << bytes;
	}

	void writeIndentation() {
		for (std::size_t i = 0; i < options.indentation; ++i) {
			write(options.indentationCharacter);
		}
	}

	void writeNull() {
		write("null");
	}

	void writeBoolean(Boolean value) {
		write((value) ? "true" : "false");
	}

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

	template <typename CharT, typename Traits>
	void writeString(std::basic_string_view<CharT, Traits> value) requires(!std::is_same_v<CharT, char> || !std::is_same_v<Traits, std::char_traits<char>>) {
		const std::span<const std::byte> bytes = std::as_bytes(std::span{value});
		writeString(std::string_view{reinterpret_cast<const char*>(bytes.data()), bytes.size()});
	}

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

	void writeNumber(Number value);

	void writeObject(const auto& value) {
		auto it = std::begin(value);
		const auto end = std::end(value);
		if (options.prettyPrint) {
			if (it == end) {
				write("{}");
			} else if (detail::getRecursiveSize(value) <= 4) {
				write("{ ");
				writeString(it->first);
				write(": ");
				serialize(it->second);
				for (++it; it != end; ++it) {
					write(", ");
					writeString(it->first);
					write(": ");
					serialize(it->second);
				}
				write(" }");
			} else {
				write("{\n");
				options.indentation += options.relativeIndentation;
				writeIndentation();
				writeString(it->first);
				write(": ");
				serialize(it->second);
				for (++it; it != end; ++it) {
					write(",\n");
					writeIndentation();
					writeString(it->first);
					write(": ");
					serialize(it->second);
				}
				write('\n');
				options.indentation -= options.relativeIndentation;
				writeIndentation();
				write('}');
			}
		} else {
			write('{');
			if (!value.empty()) {
				writeString(it->first);
				write(':');
				serialize(it->second);
				for (++it; it != end; ++it) {
					write(',');
					writeString(it->first);
					write(':');
					serialize(it->second);
				}
			}
			write('}');
		}
	}

	void writeArray(const auto& value) {
		auto it = std::begin(value);
		const auto end = std::end(value);
		if (options.prettyPrint) {
			if (it == end) {
				write("[]");
			} else if (detail::getRecursiveSize(value) <= 4) {
				write("[");
				serialize(*it);
				for (++it; it != end; ++it) {
					write(", ");
					serialize(*it);
				}
				write("]");
			} else {
				write("[\n");
				options.indentation += options.relativeIndentation;
				writeIndentation();
				serialize(*it);
				for (++it; it != end; ++it) {
					write(",\n");
					writeIndentation();
					serialize(*it);
				}
				write("\n");
				options.indentation -= options.relativeIndentation;
				writeIndentation();
				write("]");
			}
		} else {
			write("[");
			if (!value.empty()) {
				serialize(*it);
				for (++it; it != end; ++it) {
					write(",");
					serialize(*it);
				}
			}
			write("]");
		}
	}

	void writeOptional(const auto& value) {
		if (value) {
			serialize(*value);
		} else {
			writeNull();
		}
	}

	template <typename T>
	void writeAggregate(const T& value) {
		if constexpr (reflection::aggregate_size_v<T> == 0) {
			write("[]");
		} else if constexpr (reflection::aggregate_size_v<T> == 1) {
			const auto& [v] = value;
			serialize(v);
		} else if (options.prettyPrint) {
			if (detail::getRecursiveSize(value) <= 4) {
				write("[");
				bool successor = false;
				reflection::forEach(reflection::fields(value), [&](const auto& v) {
					if (successor) {
						write(", ");
					}
					successor = true;
					serialize(v);
				});
				write("]");
			} else {
				write("[\n");
				options.indentation += options.relativeIndentation;
				bool successor = false;
				reflection::forEach(reflection::fields(value), [&](const auto& v) {
					if (successor) {
						write(",\n");
					}
					successor = true;
					writeIndentation();
					serialize(v);
				});
				write("\n");
				options.indentation -= options.relativeIndentation;
				writeIndentation();
				write("]");
			}
		} else {
			write("[");
			bool successor = false;
			reflection::forEach(reflection::fields(value), [&](const auto& v) {
				if (successor) {
					write(",");
				}
				successor = true;
				serialize(v);
			});
			write("]");
		}
	}

	template <typename T>
	void serialize(const T& value) {
		json::Serializer<std::remove_cvref_t<T>>{}.serialize(*this, value);
	}

private:
	template <typename T>
	friend void serialize(std::ostream& stream, const T& value, const SerializationOptions& options);

	SerializationState(std::ostream& stream, const SerializationOptions& options)
		: stream(stream)
		, options(options) {}
};

struct DeserializationState {
private:
	using TokenType = detail::TokenType;
	using Token = detail::Token<std::istreambuf_iterator<char>>;

	detail::Parser<std::istreambuf_iterator<char>> parser;

public:
	DeserializationOptions options;

	SourceLocation readNull() {
		const auto [null, source] = parser.parseNull();
		return source;
	}

	SourceLocation readBoolean(Boolean& value) {
		const auto [boolean, source] = parser.parseBoolean();
		value = boolean;
		return source;
	}

	SourceLocation readString(String& value) {
		auto [string, source] = parser.parseString();
		value = std::move(string);
		return source;
	}

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
			std::istringstream stringStream{std::move(string)};
			json::deserialize(stringStream, value);
		}
		return source;
	}

	SourceLocation readNumber(Number& value) {
		const auto [number, source] = parser.parseNumber();
		value = number;
		return source;
	}

	template <typename T>
	SourceLocation readNumber(T& value) {
		Number number{};
		const SourceLocation source = readNumber(number);
		value = static_cast<T>(number);
		return source;
	}

	SourceLocation readObject(Object& value) {
		auto [object, source] = parser.parseObject();
		value = std::move(object);
		return source;
	}

	SourceLocation readObject(auto& value) {
		const SourceLocation source = parser.peek().source;
		if (const Token token = parser.eat(); token.type != TokenType::PUNCTUATOR_OPEN_CURLY_BRACE) {
			throw Error{"Expected an object.", token.source};
		}
		value.clear();
		if (parser.peek().type != TokenType::PUNCTUATOR_CLOSE_CURLY_BRACE) {
			while (true) {
				std::remove_cvref_t<decltype(std::begin(value)->first)> elementKey{};
				std::remove_cvref_t<decltype(std::begin(value)->second)> elementValue{};
				readString(elementKey);
				if (const Token token = parser.eat(); token.type != TokenType::PUNCTUATOR_COLON) {
					throw Error{"Expected a colon.", token.source};
				}
				deserialize(elementValue);
				value.emplace(std::move(elementKey), std::move(elementValue));
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

	SourceLocation readArray(Array& value) {
		auto [array, source] = parser.parseArray();
		value = std::move(array);
		return source;
	}

	SourceLocation readArray(auto& value) {
		const SourceLocation source = parser.peek().source;
		if (const Token token = parser.eat(); token.type != TokenType::PUNCTUATOR_OPEN_SQUARE_BRACKET) {
			throw Error{"Expected an array.", token.source};
		}
		value.clear();
		if (parser.peek().type != TokenType::PUNCTUATOR_CLOSE_SQUARE_BRACKET) {
			while (true) {
				std::remove_cvref_t<decltype(*std::begin(value))> element{};
				deserialize(element);
				value.push_back(std::move(element));
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

	SourceLocation readValue(Value& value) {
		auto [v, source] = parser.parseValue();
		value = std::move(v);
		return source;
	}

	template <typename T>
	SourceLocation readOptional(T& value) {
		const SourceLocation source = parser.peek().source;
		if (parser.peek().type == TokenType::IDENTIFIER_NULL) {
			parser.advance();
			value = T{};
		} else {
			value = std::remove_cvref_t<decltype(*value)>{};
			deserialize(*value);
		}
		return source;
	}

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

	template <typename T>
	void deserialize(T& value) {
		json::Deserializer<std::remove_cvref_t<T>>{}.deserialize(*this, value);
	}

private:
	template <typename T>
	friend void deserialize(std::istream& stream, T& value, const DeserializationOptions& options);

	DeserializationState(std::istream& stream, const DeserializationOptions& options)
		: parser({{{stream}, {}}, {}, {.lineNumber = 1, .columnNumber = 1}})
		, options(options) {}
};

template <typename T>
inline void serialize(std::ostream& stream, const T& value, const SerializationOptions& options) {
	SerializationState state{stream, options};
	state.serialize(value);
}

template <typename T>
inline void deserialize(std::istream& stream, T& value, const DeserializationOptions& options) {
	DeserializationState state{stream, options};
	state.deserialize(value);
}

namespace detail {

template <typename T>
concept nullable = !std::is_arithmetic_v<T> && requires(const T& value) {
	static_cast<bool>(value);
	static_cast<bool>(!value);
	T{};
};

template <typename T>
concept serializable_as_string = std::is_same_v<T, String> || requires(const T& value) {
	std::string_view{value};
} || requires(const T& value) {
	std::u8string_view{value};
} || requires(const T& value) {
	std::u16string_view{value};
} || requires(const T& value) {
	std::u32string_view{value};
} || requires(const T& value) {
	std::wstring_view{value};
};

template <typename T>
concept deserializable_as_string = std::is_same_v<T, String> || requires(T& value) {
	value = std::string{};
} || requires(T& value) {
	value = std::u8string{};
} || requires(T& value) {
	value = std::u16string{};
} || requires(T& value) {
	value = std::u32string{};
} || requires(T& value) {
	value = std::wstring{};
};

template <typename T>
concept serializable_as_object = std::is_same_v<T, Object> || requires(SerializationState& state, const T& value) {
	state.writeString(std::begin(value)->first);
	state.serialize(std::begin(value)->second);
};

template <typename T>
concept deserializable_as_object = std::is_same_v<T, Object> || requires(DeserializationState& state, T& value) {
	value.clear();
	std::remove_cvref_t<decltype(std::begin(value)->first)>{};
	std::remove_cvref_t<decltype(std::begin(value)->second)>{};
	state.readString(std::begin(value)->first);
	state.deserialize(std::begin(value)->second);
	value.emplace(std::remove_cvref_t<decltype(std::begin(value)->first)>{}, std::remove_cvref_t<decltype(std::begin(value)->second)>{});
};

template <typename T>
concept serializable_as_array = std::is_same_v<T, Array> || requires(SerializationState& state, const T& value) {
	state.serialize(*std::begin(value));
};

template <typename T>
concept deserializable_as_array = std::is_same_v<T, Array> || requires(DeserializationState& state, T& value) {
	value.clear();
	std::remove_cvref_t<decltype(*std::begin(value))>{};
	state.deserialize(*std::begin(value));
	value.push_back(std::remove_cvref_t<decltype(*std::begin(value))>{});
};

template <typename T>
concept serializable_as_optional = !std::is_pointer_v<T> && requires(SerializationState & state, const T& value) {
	static_cast<bool>(value);
	state.serialize(*value);
};

template <typename T>
concept deserializable_as_optional = !std::is_pointer_v<T> && requires(DeserializationState & state, T& value) {
	value = T{};
	value = std::remove_cvref_t<decltype(*value)>{};
	state.deserialize(*value);
};

template <typename T>
inline constexpr bool always_false_v = false;

template <typename T>
inline std::size_t getRecursiveSize(const T& value) {
	if constexpr (std::is_same_v<T, Value>) {
		return match(value)([&](const auto& v) -> std::size_t { return getRecursiveSize(v); });
	} else if constexpr (serializable_as_string<T>) {
		return 1;
	} else if constexpr (serializable_as_object<T>) {
		if constexpr (nullable<T>) {
			if (!value) {
				return 1;
			}
		}
		return std::accumulate(
			std::begin(value), std::end(value), std::size_t{1}, [](std::size_t count, const auto& kv) -> std::size_t { return count + getRecursiveSize(kv.second); });
	} else if constexpr (serializable_as_array<T>) {
		if constexpr (nullable<T>) {
			if (!value) {
				return 1;
			}
		}
		return std::accumulate(std::begin(value), std::end(value), std::size_t{1}, [](std::size_t count, const auto& v) -> std::size_t { return count + getRecursiveSize(v); });
	} else if constexpr (serializable_as_optional<T>) {
		return 1;
	} else if constexpr (std::is_aggregate_v<T>) {
		if constexpr (nullable<T>) {
			if (!value) {
				return 1;
			}
		}
		std::size_t result = 1;
		reflection::forEach(reflection::fields(value), [&](const auto& v) -> void { result += getRecursiveSize(v); });
		return result;
	} else {
		return 1;
	}
}

} // namespace detail

template <typename T>
struct Serializer {
	void serialize(SerializationState& state, const T& value) {
		if constexpr (detail::nullable<T>) {
			if (!value) {
				state.writeNull();
				return;
			}
		}
		if constexpr (detail::serializable_as_string<T>) {
			state.writeString(value);
		} else if constexpr (detail::serializable_as_object<T>) {
			state.writeObject(value);
		} else if constexpr (detail::serializable_as_array<T>) {
			state.writeArray(value);
		} else if constexpr (detail::serializable_as_optional<T>) {
			state.writeOptional(value);
		} else if constexpr (std::is_aggregate_v<T>) {
			state.writeAggregate(value);
		} else {
			static_assert(detail::always_false_v<T>, "JSON serialization is not implemented for the given type.");
		}
	}
};

template <typename T>
struct Deserializer {
	void deserialize(DeserializationState& state, T& value) {
		if constexpr (detail::serializable_as_string<T>) {
			state.readString(value);
		} else if constexpr (detail::serializable_as_object<T>) {
			state.readObject(value);
		} else if constexpr (detail::serializable_as_array<T>) {
			state.readArray(value);
		} else if constexpr (detail::serializable_as_optional<T>) {
			state.readOptional(value);
		} else if constexpr (std::is_aggregate_v<T>) {
			state.readAggregate(value);
		} else {
			static_assert(detail::always_false_v<T>, "JSON deserialization is not implemented for the given type.");
		}
	}
};

template <>
struct Serializer<Null> {
	void serialize(SerializationState& state, Null) {
		state.writeNull();
	}
};

template <>
struct Serializer<std::nullptr_t> {
	void serialize(SerializationState& state, std::nullptr_t) {
		state.writeNull();
	}
};

template <>
struct Serializer<Boolean> {
	void serialize(SerializationState& state, Boolean value) {
		state.writeBoolean(value);
	}
};

template <detail::number Num>
struct Serializer<Num> {
	void serialize(SerializationState& state, Num value) {
		state.writeNumber(static_cast<Number>(value));
	}
};

template <>
struct Serializer<char> {
	void serialize(SerializationState& state, char value) {
		state.writeString(std::string_view{&value, 1});
	}
};

template <>
struct Serializer<char8_t> {
	void serialize(SerializationState& state, char8_t value) {
		state.writeString(std::u8string_view{&value, 1});
	}
};

template <>
struct Serializer<char16_t> {
	void serialize(SerializationState& state, char16_t value) {
		state.writeString(std::u16string_view{&value, 1});
	}
};

template <>
struct Serializer<char32_t> {
	void serialize(SerializationState& state, char32_t value) {
		state.writeString(std::u32string_view{&value, 1});
	}
};

template <>
struct Serializer<wchar_t> {
	void serialize(SerializationState& state, wchar_t value) {
		state.writeString(std::wstring_view{&value, 1});
	}
};

template <>
struct Serializer<Value> {
	void serialize(SerializationState& state, const Value& value) {
		match(value)([&](const auto& v) -> void { state.serialize(v); });
	}
};

template <>
struct Deserializer<Null> {
	void deserialize(DeserializationState& state, Null&) {
		state.readNull();
	}
};

template <>
struct Deserializer<std::nullptr_t> {
	void deserialize(DeserializationState& state, std::nullptr_t&) {
		state.readNull();
	}
};

template <>
struct Deserializer<Boolean> {
	void deserialize(DeserializationState& state, Boolean& value) {
		state.readBoolean(value);
	}
};

template <detail::number Num>
struct Deserializer<Num> {
	void deserialize(DeserializationState& state, Num& value) {
		state.readNumber(value);
	}
};

template <>
struct Deserializer<char> {
	void deserialize(DeserializationState& state, char& value) {
		String string{};
		const SourceLocation source = state.readString(string);
		if (string.size() != 1) {
			throw Error{"Expected only a single character.", source};
		}
		value = string.front();
	}
};

template <>
struct Deserializer<char8_t> {
	void deserialize(DeserializationState& state, char8_t& value) {
		String string{};
		const SourceLocation source = state.readString(string);
		if (string.size() != sizeof(char8_t)) {
			throw Error{"Expected only a single UTF-8 code unit.", source};
		}
		std::memcpy(&value, string.data(), sizeof(char8_t));
	}
};

template <>
struct Deserializer<char16_t> {
	void deserialize(DeserializationState& state, char16_t& value) {
		String string{};
		const SourceLocation source = state.readString(string);
		if (string.size() != sizeof(char16_t)) {
			throw Error{"Expected only a single UTF-16 code unit.", source};
		}
		std::memcpy(&value, string.data(), sizeof(char16_t));
	}
};

template <>
struct Deserializer<char32_t> {
	void deserialize(DeserializationState& state, char32_t& value) {
		String string{};
		const SourceLocation source = state.readString(string);
		if (string.size() != sizeof(char32_t)) {
			throw Error{"Expected only a single UTF-32 code unit.", source};
		}
		std::memcpy(&value, string.data(), sizeof(char32_t));
	}
};

template <>
struct Deserializer<wchar_t> {
	void deserialize(DeserializationState& state, wchar_t& value) {
		String string{};
		const SourceLocation source = state.readString(string);
		if (string.size() != sizeof(wchar_t)) {
			throw Error{"Expected only a single wide character.", source};
		}
		std::memcpy(&value, string.data(), sizeof(wchar_t));
	}
};

template <>
struct Deserializer<Value> {
	void deserialize(DeserializationState& state, Value& value) {
		state.readValue(value);
	}
};

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
inline std::pair<Object::iterator, bool> Object::insert(P&& value) requires(std::is_constructible_v<value_type, P&&>) {
	return emplace(std::forward<P>(value));
}

template <typename P>
inline Object::iterator Object::insert(const_iterator pos, P&& value) requires(std::is_constructible_v<value_type, P&&>) {
	return emplace_hint(pos, std::move(value));
}

template <typename InputIt>
inline void Object::insert(InputIt first, InputIt last) {
	while (first != last) {
		insert(*first++);
	}
}

inline void Object::insert(std::initializer_list<value_type> ilist) {
	insert(ilist.begin(), ilist.end());
}

template <typename... Args>
inline std::pair<Object::iterator, bool> Object::emplace(Args&&... args) requires(std::is_constructible_v<value_type, Args...>) {
	value_type value{std::forward<Args>(args)...};
	const auto [first, last] = equal_range(value.first);
	if (first != last) {
		return {first, false};
	}
	const auto it = membersSortedByName.insert(last, std::move(value));
	return {it, true};
}

template <typename... Args>
inline Object::iterator Object::emplace_hint(const_iterator, Args&&... args) requires(std::is_constructible_v<value_type, Args...>) {
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
	return membersSortedByName <=> other.membersSortedByName;
}

template <typename Pred>
inline Object::size_type erase_if(Object& c, Pred pred) {
	return std::erase_if(c.membersSortedByName, pred);
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
	return values <=> other.values;
}

template <typename U>
inline Array::size_type erase(Array& c, const U& value) {
	return std::erase(c.values, value);
}

template <typename Pred>
inline Array::size_type erase_if(Array& c, Pred pred) {
	return std::erase_if(c.values, pred);
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

inline std::string Value::toString() const {
	std::ostringstream stream{};
	stream << *this;
	return std::move(stream).str();
}

} // namespace json
} // namespace donut

#endif
