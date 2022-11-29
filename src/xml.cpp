#include <donut/xml.hpp>

#include <array>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>

namespace donut {
namespace xml {

namespace {

class Parser {
public:
	[[nodiscard]] static constexpr bool isWhitespace(char ch) noexcept {
		return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
	}

	[[nodiscard]] static constexpr bool isNameStartCharacter(char ch) noexcept {
		return ch == ':' || (ch >= 'A' && ch <= 'Z') || ch == '_' || (ch >= 'a' && ch <= 'z');
	}

	[[nodiscard]] static constexpr bool isNameCharacter(char ch) noexcept {
		return isNameStartCharacter(ch) || ch == '-' || ch == '.' || (ch >= '0' && ch <= '9');
	}

	[[nodiscard]] static constexpr bool isValidCodePoint(std::uint32_t value) noexcept {
		return value <= 0x10FFFF && (value < 0xD800 || value > 0xDFFF);
	}

	[[nodiscard]] static constexpr int hexadecimalValue(char ch) noexcept {
		return (ch >= 'A') ? (ch >= 'a') ? ch - 'a' + 10 : ch - 'A' + 10 : ch - '0';
	}

	[[nodiscard]] static constexpr std::optional<std::uint32_t> parseCodePoint(std::string_view bytes, int radix) {
		std::int64_t prefix = 1;
		std::uint32_t result = 0;
		for (std::size_t i = bytes.size(); i-- > 0;) {
			const char ch = bytes[i];
			const int digit = hexadecimalValue(ch);
			if (digit < 0 || digit > radix) {
				return std::nullopt;
			}
			const std::int64_t digitValue = static_cast<std::int64_t>(digit) * prefix;
			if (digitValue > static_cast<std::int64_t>(std::numeric_limits<std::uint32_t>::max() - result)) {
				return std::nullopt;
			}
			result += static_cast<std::uint32_t>(digitValue);
			prefix *= radix;
		}
		if (!isValidCodePoint(result)) {
			return std::nullopt;
		}
		return result;
	}

	[[nodiscard]] static constexpr std::array<char, 5> codePointToUTF8(std::uint32_t codePoint) {
		std::array<char, 5> result{};
		if (codePoint <= 0x7F) {
			result[0] = static_cast<char>(codePoint);
		} else if (codePoint <= 0x7FF) {
			result[0] = static_cast<char>((codePoint >> 6) + 192);
			result[1] = static_cast<char>((codePoint & 63) + 128);
		} else if (codePoint <= 0xFFFF) {
			result[0] = static_cast<char>((codePoint >> 12) + 224);
			result[1] = static_cast<char>(((codePoint >> 6) & 63) + 128);
			result[2] = static_cast<char>((codePoint & 63) + 128);
		} else {
			result[0] = static_cast<char>((codePoint >> 18) + 240);
			result[1] = static_cast<char>(((codePoint >> 12) & 63) + 128);
			result[2] = static_cast<char>(((codePoint >> 6) & 63) + 128);
			result[3] = static_cast<char>((codePoint & 63) + 128);
		}
		return result;
	}

	explicit Parser(std::string_view string) noexcept
		: it(string.begin())
		, end(string.end()) {}

	void skipWhitespace() {
		while (it != end && isWhitespace(*it)) {
			++it;
		}
	}

	[[nodiscard]] bool read(char ch) {
		if (it != end && *it == ch) {
			++it;
			return true;
		}
		return false;
	}

	[[nodiscard]] bool read(std::string_view str) {
		std::string_view::iterator p = it;
		for (const char ch : str) {
			if (*p != ch) {
				return false;
			}
			++p;
		}
		it = p;
		return true;
	}

	[[nodiscard]] std::string parseName() {
		const std::string_view::iterator begin = it;
		while (it != end && isNameCharacter(*it)) {
			++it;
		}
		if (it == begin) {
			throw Error{"Missing name", it};
		}
		return std::string{begin, it};
	}

	void parseReference(std::string& output) {
		if (it == end || *it != '&') {
			throw Error{"Missing reference", it};
		}
		++it;
		if (read('#')) {
			int radix = 10;
			if (read('x')) {
				radix = 16;
			}
			const std::string_view::iterator codePointBegin = it;
			while (it != end && *it != ';') {
				++it;
			}
			const std::string_view::iterator codePointEnd = it;
			const std::optional<std::uint32_t> codePoint = parseCodePoint(std::string_view{codePointBegin, codePointEnd}, radix);
			if (!codePoint) {
				throw Error{"Invalid code point", codePointBegin};
			}
			output.append(codePointToUTF8(*codePoint).data());
		} else {
			char character{};
			if (read("amp;")) {
				character = '&';
			} else if (read("lt;")) {
				character = '<';
			} else if (read("gt;")) {
				character = '>';
			} else if (read("apos;")) {
				character = '\'';
			} else if (read("quot;")) {
				character = '\"';
			} else {
				throw Error{"Unknown reference", it};
			}
			output.push_back(character);
		}
	}

	[[nodiscard]] std::string parseQuotedString() {
		if (it == end || (*it != '\'' && *it != '\"')) {
			throw Error{"Missing quote", it};
		}
		const char quoteCharacter = *it;
		++it;
		std::string result{};
		while (it != end && !read(quoteCharacter)) {
			if (*it == '&') {
				parseReference(result);
			} else {
				result.push_back(*it);
				++it;
			}
		}
		return result;
	}

	[[nodiscard]] std::unique_ptr<Attribute> parseAttribute() {
		Attribute result{};
		result.name = parseName();
		skipWhitespace();
		if (read('=')) {
			skipWhitespace();
			result.value = parseQuotedString();
		}
		return std::make_unique<Attribute>(std::move(result));
	}

	void parseAttributes(Element& element) {
		std::unique_ptr<Attribute>* nextAttribute = &element.attributes;
		while (true) {
			skipWhitespace();
			if (it == end || *it == '>') {
				break;
			}
			if (it + 1 != end) {
				if (*it == '/' && *(it + 1) == '>') {
					break;
				}
				if (*it == '?' && *(it + 1) == '>') {
					break;
				}
			}
			*nextAttribute = parseAttribute();
			nextAttribute = &(*nextAttribute)->next;
		}
	}

	[[nodiscard]] std::unique_ptr<Element> parseElement() {
		if (!read('<')) {
			throw Error{"Missing element.", it};
		}
		Element result{};
		result.tag = parseName();
		parseAttributes(result);
		if (!read("/>")) {
			if (read("?>")) {
				throw Error{"Invalid element tag end.", it};
			}
			if (!read('>')) {
				throw Error{"Invalid tag end.", it};
			}
			parseContent(result);
		}
		return std::make_unique<Element>(std::move(result));
	}

	void commitContent(std::string& content, Element& element, std::unique_ptr<Element>*& nextChild) {
		while (!content.empty() && isWhitespace(content.back())) {
			content.pop_back();
		}
		if (!content.empty()) {
			if (element.children) {
				*nextChild = std::make_unique<Element>(Element{
					.tag{},
					.content = std::move(content),
					.attributes{},
					.children{},
					.next{},
				});
				nextChild = &(*nextChild)->next;
			} else {
				element.content = std::move(content);
			}
			content.clear();
		}
	}

	void parseContent(Element& element) {
		std::string content{};
		skipWhitespace();
		std::unique_ptr<Element>* nextChild = &element.children;
		while (true) {
			if (it == end) {
				if (!element.tag.empty()) {
					throw Error{"Missing end tag", it};
				}
				commitContent(content, element, nextChild);
				break;
			}
			if (read('<')) {
				commitContent(content, element, nextChild);
				if (read('/')) {
					const std::string tag = parseName();
					skipWhitespace();
					if (!read('>')) {
						throw Error{"Invalid end tag", it};
					}
					if (tag != element.tag) {
						throw Error{"Incorrect end tag", it};
					}
					break;
				}

				if (read('?')) {
					throw Error{"Unknown processing instruction", it};
				}

				if (read('!')) {
					if (read("--")) {
						while (it != end && !read("-->")) {
							++it;
						}
						skipWhitespace();
						continue;
					}
					throw Error{"Unknown declaration", it};
				}

				--it;
				*nextChild = parseElement();
				nextChild = &(*nextChild)->next;
				skipWhitespace();
			} else if (*it == '&') {
				parseReference(content);
			} else {
				content.push_back(*it);
				++it;
			}
		}
	}

	[[nodiscard]] std::unique_ptr<Element> parseXMLDeclarationIfPresent() {
		if (const std::string_view tag = (read("<?xml")) ? "?xml" : (read("<?XML")) ? "?XML" : ""; !tag.empty()) {
			Element declaration{.tag = std::string{tag}};
			parseAttributes(declaration);
			if (!read("?>")) {
				throw Error{"Invalid XML declaration end", it};
			}
			return std::make_unique<Element>(std::move(declaration));
		}
		return nullptr;
	}

private:
	std::string_view::iterator it;
	std::string_view::iterator end;
};

} // namespace

Document::Document(std::string_view xmlString) {
	Parser parser{xmlString};
	parser.skipWhitespace();
	declaration = parser.parseXMLDeclarationIfPresent();
	parser.skipWhitespace();
	root = parser.parseElement();
}

} // namespace xml
} // namespace donut
