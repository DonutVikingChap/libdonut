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

	Error(const std::string& message, std::string_view::iterator position)
		: std::runtime_error(message)
		, position(position) {}

	Error(const char* message, std::string_view::iterator position)
		: std::runtime_error(message)
		, position(position) {}
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
	std::unique_ptr<Element> declaration{};
	std::unique_ptr<Element> root{};

	explicit Document(std::string_view xmlString);
};

} // namespace xml
} // namespace donut

#endif
