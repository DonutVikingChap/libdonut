#ifndef DONUT_XML_HPP
#define DONUT_XML_HPP

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

namespace donut {
namespace xml {

struct Error : std::runtime_error {
	std::string_view::iterator position;
	std::size_t lineNumber;

	Error(const std::string& message, std::string_view::iterator position, std::size_t lineNumber)
		: std::runtime_error(message)
		, position(position)
		, lineNumber(lineNumber) {}

	Error(const char* message, std::string_view::iterator position, std::size_t lineNumber)
		: std::runtime_error(message)
		, position(position)
		, lineNumber(lineNumber) {}
};

struct Attribute {
	std::string name{};
	std::string value{};
	std::unique_ptr<Attribute> next{};
};

struct Element {
	std::string tag{};
	std::string content{};
	std::unique_ptr<Attribute> attributes{};
	std::unique_ptr<Element> children{};
	std::unique_ptr<Element> next{};
};

struct Document {
	[[nodiscard]] static Document parse(std::string_view xmlString);

	std::unique_ptr<Element> declaration{};
	std::unique_ptr<Element> root{};
};

} // namespace xml
} // namespace donut

#endif
