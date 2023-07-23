#ifndef DONUT_AUDIO_ERROR_HPP
#define DONUT_AUDIO_ERROR_HPP

#include <stdexcept>   // std::runtime_error
#include <string>      // std::string
#include <string_view> // std::string_view

namespace donut::audio {

/**
 * Exception type for domain-specific errors originating from the
 * donut::audio module.
 */
struct Error : std::runtime_error {
	explicit Error(const std::string& message)
		: std::runtime_error(message) {}

	explicit Error(const char* message)
		: std::runtime_error(message) {}

	Error(std::string_view message, unsigned errorCode);
};

} // namespace donut::audio

#endif
