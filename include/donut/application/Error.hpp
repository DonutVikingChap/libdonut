#ifndef DONUT_APPLICATION_ERROR_HPP
#define DONUT_APPLICATION_ERROR_HPP

#include <stdexcept>
#include <string>

namespace donut {
namespace application {

struct Error : std::runtime_error {
	explicit Error(const std::string& message)
		: std::runtime_error(message) {}

	explicit Error(const char* message)
		: std::runtime_error(message) {}
};

} // namespace application
} // namespace donut

#endif
