#include <donut/audio/Error.hpp>

#include <fmt/format.h>
#include <soloud.h>
#include <stdexcept>
#include <string_view>
#include <type_traits>

namespace donut {
namespace audio {

namespace {

constexpr std::string_view getErrorCodeMessage(unsigned errorCode) noexcept {
	static_assert(std::is_same_v<unsigned, SoLoud::result>);
	const char* message = "Unknown error.";
	switch (errorCode) {
		case SoLoud::SO_NO_ERROR: message = "No error."; break;
		case SoLoud::INVALID_PARAMETER: message = "Some parameter is invalid."; break;
		case SoLoud::FILE_NOT_FOUND: message = "File not found."; break;
		case SoLoud::FILE_LOAD_FAILED: message = "File found, but could not be loaded."; break;
		case SoLoud::DLL_NOT_FOUND: message = "DLL not found, or wrong DLL."; break;
		case SoLoud::OUT_OF_MEMORY: message = "Out of memory."; break;
		case SoLoud::NOT_IMPLEMENTED: message = "Feature not implemented."; break;
		default: break;
	}
	return message;
}

} // namespace

Error::Error(std::string_view message, unsigned errorCode)
	: std::runtime_error(fmt::format("{}: {}", message, getErrorCodeMessage(errorCode))) {}

} // namespace audio
} // namespace donut
