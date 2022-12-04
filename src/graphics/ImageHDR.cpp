#include <donut/graphics/ImageHDR.hpp>

#include <cstddef>  // std::size_t
#include <cstring>  // std::memcpy
#include <stdlib.h> // malloc // NOLINT(modernize-deprecated-headers)

namespace donut {
namespace graphics {

ImageHDR::ImageHDR(const float* pixels, std::size_t width, std::size_t height, std::size_t channelCount)
	: Image(Image::Pixels{malloc(width * height * channelCount * sizeof(float))}, // NOLINT(cppcoreguidelines-no-malloc)
		  width, height, channelCount) {
	std::memcpy(getPixels(), pixels, width * height * channelCount * sizeof(float));
}

} // namespace graphics
} // namespace donut
