#include <donut/OutputFileStream.hpp>
#include <donut/graphics/Error.hpp>
#include <donut/graphics/ImageHDR.hpp>

#include <cstddef>           // std::size_t, std::byte
#include <cstring>           // std::memcpy
#include <fmt/format.h>      // fmt::format
#include <new>               // std::bad_alloc
#include <span>              // std::span
#include <stb_image_write.h> // stbi_..., stbi_write_...
#include <stdlib.h>          // malloc // NOLINT(modernize-deprecated-headers)

namespace donut {
namespace graphics {

namespace {

void imageFileOutputCallback(void* context, void* data, int size) noexcept {
	OutputFileStream& file = *static_cast<OutputFileStream*>(context);
	file.write(std::span{reinterpret_cast<const std::byte*>(data), static_cast<std::size_t>(size)});
}

} // namespace

void ImageHDR::saveHDR(const ImageHDRView& image, const char* filepath, const ImageHDRSaveHDROptions& options) {
	OutputFileStream file = OutputFileStream::create(filepath);
	stbi_flip_vertically_on_write(options.flipVertically ? 1 : 0);
	const int width = static_cast<int>(image.getWidth());
	const int height = static_cast<int>(image.getHeight());
	const int channelCount = static_cast<int>(image.getChannelCount());
	const float* const pixels = static_cast<const float*>(image.getPixels());
	if (stbi_write_hdr_to_func(imageFileOutputCallback, &file, width, height, channelCount, pixels) == 0) {
		throw Error{fmt::format("Failed to save HDR image \"{}\"!", filepath)};
	}
}

ImageHDR::ImageHDR(const float* pixels, std::size_t width, std::size_t height, std::size_t channelCount)
	: Image(Image::Pixels{malloc(width * height * channelCount * sizeof(float))}, // NOLINT(cppcoreguidelines-no-malloc)
		  width, height, channelCount) {
	if (!getPixels()) {
		throw std::bad_alloc{};
	}
	std::memcpy(getPixels(), pixels, width * height * channelCount * sizeof(float));
}

} // namespace graphics
} // namespace donut
