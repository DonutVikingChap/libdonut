#ifndef DONUT_XML_HPP
#define DONUT_XML_HPP

#include <cstddef>     // std::size_t
#include <memory>      // std::unique_ptr
#include <stdexcept>   // std::runtime_error
#include <string>      // std::string
#include <string_view> // std::string_view

namespace donut {
namespace xml {

/**
 * Exception type for errors originating from the XML API.
 */
struct Error : std::runtime_error {
	/**
	 * Iterator into the source XML string where the error originated from.
	 */
	std::string_view::iterator position;

	/**
	 * Line number, starting at 1 for the first line, where the error occured.
	 */
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

/**
 * Named attribute of an Element with an optional value.
 */
struct Attribute {
	std::string name{};                ///< Name of the attribute.
	std::string value{};               ///< Attribute value, or empty for no value.
	std::unique_ptr<Attribute> next{}; ///< Next neighboring attribute in the list that this attribute is part of.
};

/**
 * Node in a Document.
 */
struct Element {
	std::string tag{};                       ///< Element tag name.
	std::string content{};                   ///< Raw non-element text content of the element.
	std::unique_ptr<Attribute> attributes{}; ///< Linked list of element attributes.
	std::unique_ptr<Element> children{};     ///< Linked list of children of this element.
	std::unique_ptr<Element> next{};         ///< Next neighboring element in the list that this element is part of.
};

/**
 * Tree of Element nodes defined by an XML file.
 */
struct Document {
	/**
	 * Parse a document form an XML string.
	 *
	 * \param xmlString read-only view over the XML string to parse.
	 *
	 * \return the parsed document.
	 *
	 * \throws Error on failure to parse any element of the document.
	 * \throws std::bad_alloc on allocation failure.
	 */
	[[nodiscard]] static Document parse(std::string_view xmlString);

	std::unique_ptr<Element> declaration{}; ///< Optional XML declaration.
	std::unique_ptr<Element> root{};        ///< Root element of the document tree.
};

} // namespace xml
} // namespace donut

#endif
