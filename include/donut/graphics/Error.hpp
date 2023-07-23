#ifndef DONUT_GRAPHICS_ERROR_HPP
#define DONUT_GRAPHICS_ERROR_HPP

#include <stdexcept> // std::runtime_error
#include <string>    // std::string

namespace donut::graphics {

/**
 * Exception type for domain-specific errors originating from the
 * donut::graphics module.
 */
struct Error : std::runtime_error {
	explicit Error(const std::string& message)
		: std::runtime_error(message) {}

	explicit Error(const char* message)
		: std::runtime_error(message) {}
};

} // namespace donut::graphics

#endif
