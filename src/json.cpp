#include <donut/json.hpp>
#include <donut/unicode.hpp>

#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <iterator>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

namespace donut {
namespace json {

namespace detail {

template <typename It>
Lexer<It>::Lexer(unicode::UTF8Iterator<It> it, unicode::UTF8Sentinel end, const SourceLocation& source)
	: it(std::move(it))
	, end(end)
	, source(source) {
	if (this->it == this->end) {
		currentCodePoint = std::nullopt;
	} else {
		currentCodePoint = *this->it;
		++this->it;
	}
}

template <typename It>
Token<It> Lexer<It>::scan() {
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

template <typename It>
void Lexer<It>::skipWhitespace() {
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

template <typename It>
void Lexer<It>::skipLineTerminatorSequence() {
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

template <typename It>
void Lexer<It>::advance() {
	if (it != end) {
		currentCodePoint = *it;
		++it;
	} else {
		currentCodePoint = std::nullopt;
	}
	++source.columnNumber;
}

template <typename It>
bool Lexer<It>::hasReachedEnd() const noexcept {
	return !currentCodePoint;
}

template <typename It>
char32_t Lexer<It>::peek() const {
	return *currentCodePoint;
}

template <typename It>
std::optional<char32_t> Lexer<It>::lookahead() const {
	if (it != end) {
		return *it;
	}
	return std::nullopt;
}

template <typename It>
Token<It> Lexer<It>::scanPunctuator() {
	String string{static_cast<char>(*it)};
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
	return {.string = std::move(string), .source = source, .type = type};
}

template <typename It>
Token<It> Lexer<It>::scanString() {
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
			const unicode::UTF8FromCodePointResult codePointUTF8 = unicode::getUTF8FromCodePoint(peek());
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
				const unicode::UTF8FromCodePointResult codePointUTF8 = unicode::getUTF8FromCodePoint(peek());
				string.append(std::string_view{reinterpret_cast<const char*>(codePointUTF8.codeUnits.data()), codePointUTF8.size});
				break;
			}
		}
		advance();
	}
	throw Error{"Missing end of string quote character.", source};
}

template <typename It>
Token<It> Lexer<It>::scanNumber() {
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

template <typename It>
Token<It> Lexer<It>::scanIdentifier() {
	String string{};
	const SourceLocation identifierSource = source;
	do {
		if (!unicode::isValidCodePoint(peek())) {
			throw Error{"Invalid UTF-8.", source};
		}
		const unicode::UTF8FromCodePointResult codePointUTF8 = unicode::getUTF8FromCodePoint(peek());
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

template <typename It>
void Lexer<It>::scanNumericEscapeSequence(String& output, std::size_t minDigitCount, std::size_t maxDigitCount, int radix, bool (*isDigit)(char32_t) noexcept) {
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
		parseResult.ec != std::errc{} || parseResult.ptr != digitsEnd || !unicode::isValidCodePoint(codePointValue)) {
		throw Error{"Invalid code point value.", escapeSequenceSource};
	}
	const unicode::UTF8FromCodePointResult codePointUTF8 = unicode::getUTF8FromCodePoint(static_cast<char32_t>(codePointValue));
	output.append(std::string_view{reinterpret_cast<const char*>(codePointUTF8.codeUnits.data()), codePointUTF8.size});
}

template class Lexer<const char8_t*>;
template class Lexer<std::istream_iterator<char>>;

template <typename It>
Parser<It>::Parser(Lexer<It> lexer)
	: lexer(std::move(lexer))
	, currentToken(this->lexer.scan()) {}

template <typename It>
Value Parser<It>::parseFile() {
	Value result = parseValue().first;
	if (const Token<It>& token = peek(); token.type != TokenType::END_OF_FILE) {
		throw Error{"Multiple top-level values.", token.source};
	}
	return result;
}

template <typename It>
std::pair<Null, SourceLocation> Parser<It>::parseNull() {
	switch (const Token<It> token = eat(); token.type) {
		case TokenType::IDENTIFIER_NULL: return {Null{}, token.source};
		default: throw Error{"Expected a null.", token.source};
	}
	return {};
}

template <typename It>
std::pair<Boolean, SourceLocation> Parser<It>::parseBoolean() {
	switch (const Token<It> token = eat(); token.type) {
		case TokenType::IDENTIFIER_FALSE: return {Boolean{false}, token.source};
		case TokenType::IDENTIFIER_TRUE: return {Boolean{true}, token.source};
		default: throw Error{"Expected a boolean.", token.source};
	}
	return {};
}

template <typename It>
std::pair<String, SourceLocation> Parser<It>::parseString() {
	switch (Token<It> token = eat(); token.type) {
		case TokenType::STRING: return {String{std::move(token.string)}, token.source};
		default: throw Error{"Expected a string.", token.source};
	}
	return {};
}

template <typename It>
std::pair<Number, SourceLocation> Parser<It>::parseNumber() {
	switch (Token<It> token = eat(); token.type) {
		case TokenType::NUMBER_BINARY: return {parseNumberContents(std::move(token), 2), token.source};
		case TokenType::NUMBER_OCTAL: return {parseNumberContents(std::move(token), 8), token.source};
		case TokenType::NUMBER_DECIMAL: return {parseNumberContents(std::move(token), 10), token.source};
		case TokenType::NUMBER_HEXADECIMAL: return {parseNumberContents(std::move(token), 16), token.source};
		case TokenType::NUMBER_POSITIVE_INFINITY: return {Number{std::numeric_limits<Number>::infinity()}, token.source};
		case TokenType::NUMBER_NEGATIVE_INFINITY: return {Number{-std::numeric_limits<Number>::infinity()}, token.source};
		case TokenType::NUMBER_POSITIVE_NAN: return {Number{std::numeric_limits<Number>::quiet_NaN()}, token.source};
		case TokenType::NUMBER_NEGATIVE_NAN: return {Number{-std::numeric_limits<Number>::quiet_NaN()}, token.source};
		default: throw Error{"Expected a number.", token.source};
	}
	return {};
}

template <typename It>
std::pair<Object, SourceLocation> Parser<It>::parseObject() {
	switch (const Token<It> token = eat(); token.type) {
		case TokenType::PUNCTUATOR_OPEN_CURLY_BRACE: return {parseObjectContents(), token.source};
		default: throw Error{"Expected an object.", token.source};
	}
	return {};
}

template <typename It>
std::pair<Array, SourceLocation> Parser<It>::parseArray() {
	switch (const Token<It> token = eat(); token.type) {
		case TokenType::PUNCTUATOR_OPEN_SQUARE_BRACKET: return {parseArrayContents(), token.source};
		default: throw Error{"Expected an array.", token.source};
	}
	return {};
}

template <typename It>
std::pair<Value, SourceLocation> Parser<It>::parseValue() {
	switch (Token<It> token = eat(); token.type) {
		case TokenType::END_OF_FILE: throw Error{"Expected a value.", token.source};
		case TokenType::IDENTIFIER_NULL: return {Null{}, token.source};
		case TokenType::IDENTIFIER_FALSE: return {Boolean{false}, token.source};
		case TokenType::IDENTIFIER_TRUE: return {Boolean{true}, token.source};
		case TokenType::IDENTIFIER_NAME: throw Error{"Unexpected name identifier.", token.source};
		case TokenType::PUNCTUATOR_COMMA: throw Error{"Unexpected comma.", token.source};
		case TokenType::PUNCTUATOR_COLON: throw Error{"Unexpected colon.", token.source};
		case TokenType::PUNCTUATOR_OPEN_SQUARE_BRACKET: return {parseArrayContents(), token.source};
		case TokenType::PUNCTUATOR_CLOSE_SQUARE_BRACKET: throw Error{"Unexpected closing bracket.", token.source};
		case TokenType::PUNCTUATOR_OPEN_CURLY_BRACE: return {parseObjectContents(), token.source};
		case TokenType::PUNCTUATOR_CLOSE_CURLY_BRACE: throw Error{"Unexpected closing brace.", token.source};
		case TokenType::STRING: return {Value{std::move(token.string)}, token.source};
		case TokenType::NUMBER_BINARY: return {parseNumberContents(std::move(token), 2), token.source};
		case TokenType::NUMBER_OCTAL: return {parseNumberContents(std::move(token), 8), token.source};
		case TokenType::NUMBER_DECIMAL: return {parseNumberContents(std::move(token), 10), token.source};
		case TokenType::NUMBER_HEXADECIMAL: return {parseNumberContents(std::move(token), 16), token.source};
		case TokenType::NUMBER_POSITIVE_INFINITY: return {Number{std::numeric_limits<Number>::infinity()}, token.source};
		case TokenType::NUMBER_NEGATIVE_INFINITY: return {Number{-std::numeric_limits<Number>::infinity()}, token.source};
		case TokenType::NUMBER_POSITIVE_NAN: return {Number{std::numeric_limits<Number>::quiet_NaN()}, token.source};
		case TokenType::NUMBER_NEGATIVE_NAN: return {Number{-std::numeric_limits<Number>::quiet_NaN()}, token.source};
	}
	return {};
}

template <typename It>
void Parser<It>::advance() {
	currentToken = lexer.scan();
}

template <typename It>
const Token<It>& Parser<It>::peek() const noexcept {
	return currentToken;
}

template <typename It>
Token<It> Parser<It>::eat() {
	Token<It> result = std::move(currentToken);
	currentToken = lexer.scan();
	return result;
}

template <typename It>
Number Parser<It>::parseNumberContents(Token<It> token, int radix) {
	const char* numberStringBegin = token.string.c_str();
	char* const numberStringEnd = token.string.data() + token.string.size();
	char* endPtr = numberStringEnd;
	if (radix == 10) {
		const double numberValue = std::strtod(numberStringBegin, &endPtr);
		if (endPtr != numberStringEnd) {
			throw Error{"Invalid number.", token.source};
		}
		return Number{numberValue};
	}
	bool negative = false;
	if (!token.string.empty() && token.string.front() == '-') {
		negative = true;
		++numberStringBegin;
	}
	const unsigned long long integerNumberValue = std::strtoull(numberStringBegin, &endPtr, radix);
	if (endPtr != numberStringEnd) {
		throw Error{"Invalid number.", token.source};
	}
	const double numberValue = static_cast<double>(integerNumberValue);
	return Number{(negative) ? -numberValue : numberValue};
}

template <typename It>
Object Parser<It>::parseObjectContents() {
	Object result{};
	Token<It> token = eat();
	while (true) {
		std::optional<String> memberName{};
		switch (token.type) {
			case TokenType::END_OF_FILE: throw Error{"Missing end of object.", token.source};
			case TokenType::IDENTIFIER_NULL: throw Error{"Unexpected null.", token.source};
			case TokenType::IDENTIFIER_FALSE: throw Error{"Unexpected false.", token.source};
			case TokenType::IDENTIFIER_TRUE: throw Error{"Unexpected true.", token.source};
			case TokenType::IDENTIFIER_NAME: [[fallthrough]];
			case TokenType::STRING: memberName = std::move(token.string); break;
			case TokenType::PUNCTUATOR_COMMA: [[fallthrough]];
			case TokenType::PUNCTUATOR_COLON: [[fallthrough]];
			case TokenType::PUNCTUATOR_OPEN_SQUARE_BRACKET: [[fallthrough]];
			case TokenType::PUNCTUATOR_CLOSE_SQUARE_BRACKET: [[fallthrough]];
			case TokenType::PUNCTUATOR_OPEN_CURLY_BRACE: throw Error{"Unexpected punctuator.", token.source};
			case TokenType::PUNCTUATOR_CLOSE_CURLY_BRACE: break;
			case TokenType::NUMBER_BINARY: [[fallthrough]];
			case TokenType::NUMBER_OCTAL: [[fallthrough]];
			case TokenType::NUMBER_DECIMAL: [[fallthrough]];
			case TokenType::NUMBER_HEXADECIMAL: [[fallthrough]];
			case TokenType::NUMBER_POSITIVE_INFINITY: [[fallthrough]];
			case TokenType::NUMBER_NEGATIVE_INFINITY: [[fallthrough]];
			case TokenType::NUMBER_POSITIVE_NAN: [[fallthrough]];
			case TokenType::NUMBER_NEGATIVE_NAN: throw Error{"Unexpected number.", token.source};
		}
		if (!memberName) {
			break;
		}
		token = eat();
		if (token.type != TokenType::PUNCTUATOR_COLON) {
			throw Error{"Expected a colon.", token.source};
		}
		auto [memberValue, memberSource] = parseValue();
		result.emplace(std::move(*memberName), std::move(memberValue));
		token = eat();
		if (token.type == TokenType::PUNCTUATOR_COMMA) {
			token = eat();
		} else if (token.type == TokenType::PUNCTUATOR_CLOSE_CURLY_BRACE) {
			break;
		} else {
			throw Error{"Expected a comma or closing brace.", token.source};
		}
	}
	return result;
}

template <typename It>
Array Parser<It>::parseArrayContents() {
	Array result{};
	Token<It> token = eat();
	while (true) {
		std::optional<Value> value{};
		switch (token.type) {
			case TokenType::END_OF_FILE: throw Error{"Missing end of object.", token.source};
			case TokenType::IDENTIFIER_NULL: value.emplace(Null{}); break;
			case TokenType::IDENTIFIER_FALSE: value.emplace(Boolean{false}); break;
			case TokenType::IDENTIFIER_TRUE: value.emplace(Boolean{true}); break;
			case TokenType::IDENTIFIER_NAME: throw Error{"Unexpected name identifier.", token.source};
			case TokenType::PUNCTUATOR_COMMA: [[fallthrough]];
			case TokenType::PUNCTUATOR_COLON: [[fallthrough]];
			case TokenType::PUNCTUATOR_CLOSE_CURLY_BRACE: throw Error{"Unexpected punctuator.", token.source};
			case TokenType::PUNCTUATOR_OPEN_SQUARE_BRACKET: value.emplace(parseArrayContents()); break;
			case TokenType::PUNCTUATOR_OPEN_CURLY_BRACE: value.emplace(parseObjectContents()); break;
			case TokenType::PUNCTUATOR_CLOSE_SQUARE_BRACKET: break;
			case TokenType::STRING: value.emplace(std::move(token.string)); break;
			case TokenType::NUMBER_BINARY: value.emplace(parseNumberContents(std::move(token), 2)); break;
			case TokenType::NUMBER_OCTAL: value.emplace(parseNumberContents(std::move(token), 8)); break;
			case TokenType::NUMBER_DECIMAL: value.emplace(parseNumberContents(std::move(token), 10)); break;
			case TokenType::NUMBER_HEXADECIMAL: value.emplace(parseNumberContents(std::move(token), 16)); break;
			case TokenType::NUMBER_POSITIVE_INFINITY: value.emplace(Number{std::numeric_limits<Number>::infinity()}); break;
			case TokenType::NUMBER_NEGATIVE_INFINITY: value.emplace(Number{-std::numeric_limits<Number>::infinity()}); break;
			case TokenType::NUMBER_POSITIVE_NAN: value.emplace(Number{std::numeric_limits<Number>::quiet_NaN()}); break;
			case TokenType::NUMBER_NEGATIVE_NAN: value.emplace(Number{-std::numeric_limits<Number>::quiet_NaN()}); break;
		}
		if (!value) {
			break;
		}
		result.push_back(std::move(*value));
		token = eat();
		if (token.type == TokenType::PUNCTUATOR_COMMA) {
			token = eat();
		} else if (token.type == TokenType::PUNCTUATOR_CLOSE_SQUARE_BRACKET) {
			break;
		} else {
			throw Error{"Expected a comma or closing bracket.", token.source};
		}
	}
	return result;
}

template class Parser<const char8_t*>;
template class Parser<std::istream_iterator<char>>;

} // namespace detail

void SerializationState::writeNumber(Number value) {
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
		[[likely]] fmt::print(stream, "{}", value);
	}
}

Value Value::parse(std::u8string_view jsonString) {
	unicode::UTF8View codePoints{jsonString};
	return detail::Parser<const char8_t*>{{codePoints.begin(), codePoints.end(), {.lineNumber = 1, .columnNumber = 1}}}.parseFile();
}

Value Value::parse(std::string_view jsonString) {
	static_assert(sizeof(char) == sizeof(char8_t));
	static_assert(alignof(char) == alignof(char8_t));
	return parse(std::u8string_view{reinterpret_cast<const char8_t*>(jsonString.data()), jsonString.size()});
}

} // namespace json
} // namespace donut
