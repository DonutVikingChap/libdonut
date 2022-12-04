#ifndef DONUT_GRAPHICS_IMAGE_HDR_HPP
#define DONUT_GRAPHICS_IMAGE_HDR_HPP

#include <donut/graphics/Image.hpp>

#include <cstddef> // std::size_t

namespace donut {
namespace graphics {

class ImageHDRView : public ImageView {
public:
	constexpr ImageHDRView() noexcept = default;

	constexpr ImageHDRView(const float* pixels, std::size_t width, std::size_t height, std::size_t channelCount) noexcept
		: ImageView(pixels, width, height, channelCount) {}

	[[nodiscard]] constexpr const float* getPixels() const noexcept {
		return static_cast<const float*>(ImageView::getPixels());
	}
};

struct ImageHDROptions {
	int desiredChannelCount = 0;
	bool flipVertically = false;
};

class ImageHDR : public Image {
public:
	ImageHDR() noexcept = default;

	explicit ImageHDR(const char* filepath, const ImageHDROptions& options = {})
		: Image(filepath,
			  {
				  .desiredChannelCount = options.desiredChannelCount,
				  .highDynamicRange = true,
				  .flipVertically = options.flipVertically,
			  }) {}

	ImageHDR(const float* pixels, std::size_t width, std::size_t height, std::size_t channelCount);

	operator ImageHDRView() const noexcept {
		return ImageHDRView{getPixels(), getWidth(), getHeight(), getChannelCount()};
	}

	[[nodiscard]] float* getPixels() noexcept {
		return static_cast<float*>(Image::getPixels());
	}

	[[nodiscard]] const float* getPixels() const noexcept {
		return static_cast<const float*>(Image::getPixels());
	}
};

} // namespace graphics
} // namespace donut

#endif
