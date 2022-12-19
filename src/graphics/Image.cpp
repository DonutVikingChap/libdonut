#include <donut/InputFileStream.hpp>
#include <donut/graphics/Error.hpp>
#include <donut/graphics/Image.hpp>

#include <cstddef>      // std::size_t, std::ptrdiff_t, std::byte
#include <fmt/format.h> // fmt::format
#include <span>         // std::span, std::as_writable_bytes
#include <stb_image.h>  // stbi_...

namespace donut {
namespace graphics {

namespace {

constexpr stbi_io_callbacks IMAGE_FILE_INPUT_CALLBACKS{
	.read = [](void* user, char* data, int size) -> int {
		InputFileStream& file = *static_cast<InputFileStream*>(user);
		return static_cast<int>(file.read(std::as_writable_bytes(std::span{data, static_cast<std::size_t>(size)})));
	},
	.skip = [](void* user, int n) -> void {
		InputFileStream& file = *static_cast<InputFileStream*>(user);
		file.skip(static_cast<std::ptrdiff_t>(n));
	},
	.eof = [](void* user) -> int {
		InputFileStream& file = *static_cast<InputFileStream*>(user);
		return (file.eof()) ? 1 : 0;
	},
};

} // namespace

Image::Image(const char* filepath, const ImageOptions& options) {
	stbi_set_flip_vertically_on_load_thread(options.flipVertically ? 1 : 0);
	int imageWidth = 0;
	int imageHeight = 0;
	int channelsInFile = 0;
	InputFileStream file = InputFileStream::open(filepath);
	if (options.highDynamicRange) {
		pixels.reset(stbi_loadf_from_callbacks(&IMAGE_FILE_INPUT_CALLBACKS, &file, &imageWidth, &imageHeight, &channelsInFile, static_cast<int>(options.desiredChannelCount)));
		if (!pixels) {
			throw Error{fmt::format("Failed to load HDR image \"{}\": {}", filepath, stbi_failure_reason())};
		}
	} else {
		pixels.reset(stbi_load_from_callbacks(&IMAGE_FILE_INPUT_CALLBACKS, &file, &imageWidth, &imageHeight, &channelsInFile, static_cast<int>(options.desiredChannelCount)));
		if (!pixels) {
			throw Error{fmt::format("Failed to load image \"{}\": {}", filepath, stbi_failure_reason())};
		}
	}
	width = static_cast<std::size_t>(imageWidth);
	height = static_cast<std::size_t>(imageHeight);
	channelCount = (options.desiredChannelCount > 0) ? static_cast<std::size_t>(options.desiredChannelCount) : static_cast<std::size_t>(channelsInFile);
}

void Image::PixelsDeleter::operator()(void* handle) const noexcept {
	stbi_image_free(handle);
}

} // namespace graphics
} // namespace donut
