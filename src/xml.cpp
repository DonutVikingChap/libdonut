#include <donut/unicode.hpp>
#include <donut/xml.hpp>

#include <charconv>     // std::from_chars_result, std::from_chars
#include <cstddef>      // std::size_t
#include <cstdint>      // std::uint32_t
#include <memory>       // std::unique_ptr, std::make_unique, std::to_address
#include <string>       // std::string
#include <string_view>  // std::string_view
#include <system_error> // std::errc
#include <utility>      // std::move

namespace donut::xml {

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

	Parser(std::string_view string, std::size_t lineNumber) noexcept
		: it(string.begin())
		, end(string.end())
		, lineNumber(lineNumber) {}

	[[nodiscard]] std::unique_ptr<Element> parseXMLDeclarationIfPresent() {
		if (const std::string_view tag = (read("<?xml")) ? "?xml" : (read("<?XML")) ? "?XML" : ""; !tag.empty()) {
			Element declaration{.tag = std::string{tag}};
			parseAttributes(declaration);
			if (!read("?>")) {
				throw Error{"Invalid XML declaration end.", it, lineNumber};
			}
			return std::make_unique<Element>(std::move(declaration));
		}
		return nullptr;
	}

	[[nodiscard]] std::unique_ptr<Element> parseElement() {
		if (!read('<')) {
			throw Error{"Missing element.", it, lineNumber};
		}
		Element result{};
		result.tag = parseName();
		parseAttributes(result);
		if (!read("/>")) {
			if (read("?>")) {
				throw Error{"Invalid element tag end.", it, lineNumber};
			}
			if (!read('>')) {
				throw Error{"Invalid tag end.", it, lineNumber};
			}
			parseContent(result);
		}
		return std::make_unique<Element>(std::move(result));
	}

	void skipWhitespace() {
		while (it != end) {
			if (*it == '\n') {
				++it;
				++lineNumber;
			} else if (*it == '\r') {
				++it;
				if (it != end && *it == '\n') {
					++it;
				}
				++lineNumber;
			} else if (*it == ' ' || *it == '\t') {
				++it;
			} else {
				break;
			}
		}
	}

private:
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
			throw Error{"Missing name.", it, lineNumber};
		}
		return std::string{begin, it};
	}

	void parseReference(std::string& output) {
		if (it == end || *it != '&') {
			throw Error{"Missing reference.", it, lineNumber};
		}
		++it;
		if (read('#')) {
			const std::string_view::iterator codePointStringBegin = it;
			int radix = 10;
			if (read('x')) {
				radix = 16;
			}
			const char* const codePointBegin = std::to_address(it);
			while (it != end && *it != ';') {
				if (*it == '\n') {
					++it;
					++lineNumber;
				} else if (*it == '\r') {
					++it;
					if (it != end && *it == '\n') {
						++it;
					}
					++lineNumber;
				} else {
					++it;
				}
			}
			const char* const codePointEnd = std::to_address(it);
			std::uint32_t codePointValue = 0;
			if (const std::from_chars_result parseResult = std::from_chars(codePointBegin, codePointEnd, codePointValue, radix);
				parseResult.ec != std::errc{} || parseResult.ptr != codePointEnd || !unicode::isValidCodePoint(static_cast<char32_t>(codePointValue))) {
				throw Error{"Invalid code point.", codePointStringBegin, lineNumber};
			}
			const unicode::EncodeUTF8FromCodePointResult codePointUTF8 = unicode::encodeUTF8FromCodePoint(static_cast<char32_t>(codePointValue));
			output.append(std::string_view{reinterpret_cast<const char*>(codePointUTF8.codeUnits.data()), codePointUTF8.size});
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
				throw Error{"Unknown reference.", it, lineNumber};
			}
			output.push_back(character);
		}
	}

	[[nodiscard]] std::string parseQuotedString() {
		if (it == end || (*it != '\'' && *it != '\"')) {
			throw Error{"Missing quote.", it, lineNumber};
		}
		const char quoteCharacter = *it;
		++it;
		std::string result{};
		while (it != end && !read(quoteCharacter)) {
			if (*it == '&') {
				parseReference(result);
			} else if (*it == '\n') {
				result.push_back(*it++);
				++lineNumber;
			} else if (*it == '\r') {
				result.push_back(*it++);
				if (it != end && *it == '\n') {
					result.push_back(*it++);
				}
				++lineNumber;
			} else {
				result.push_back(*it++);
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
					throw Error{"Missing end tag.", it, lineNumber};
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
						throw Error{"Invalid end tag.", it, lineNumber};
					}
					if (tag != element.tag) {
						throw Error{"Incorrect end tag.", it, lineNumber};
					}
					break;
				}

				if (read('?')) {
					throw Error{"Unknown processing instruction.", it, lineNumber};
				}

				if (read('!')) {
					if (read("--")) {
						while (it != end && !read("-->")) {
							++it;
						}
						skipWhitespace();
						continue;
					}
					throw Error{"Unknown declaration.", it, lineNumber};
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

	std::string_view::iterator it;
	std::string_view::iterator end;
	std::size_t lineNumber;
};

} // namespace

Document Document::parse(std::string_view xmlString) {
	Document result{};
	Parser parser{xmlString, 1};
	parser.skipWhitespace();
	result.declaration = parser.parseXMLDeclarationIfPresent();
	parser.skipWhitespace();
	result.root = parser.parseElement();
	return result;
}

} // namespace donut::xml
