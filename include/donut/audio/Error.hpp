#ifndef DONUT_AUDIO_ERROR_HPP
#define DONUT_AUDIO_ERROR_HPP

#include <stdexcept>
#include <string>
#include <string_view>

namespace donut {
namespace audio {

struct Error : std::runtime_error {
	explicit Error(const std::string& message)
		: std::runtime_error(message) {}

	explicit Error(const char* message)
		: std::runtime_error(message) {}

	Error(std::string_view message, unsigned errorCode);
};

} // namespace audio
} // namespace donut

#endif
