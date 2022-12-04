#include <donut/graphics/ImageLDR.hpp>

#include <cstddef>  // std::size_t
#include <cstring>  // std::memcpy
#include <stdlib.h> // malloc // NOLINT(modernize-deprecated-headers)

namespace donut {
namespace graphics {

ImageLDR::ImageLDR(const std::byte* pixels, std::size_t width, std::size_t height, std::size_t channelCount)
	: Image(Image::Pixels{malloc(width * height * channelCount * sizeof(std::byte))}, // NOLINT(cppcoreguidelines-no-malloc)
		  width, height, channelCount) {
	std::memcpy(getPixels(), pixels, width * height * channelCount * sizeof(std::byte));
}

} // namespace graphics
} // namespace donut
