#ifndef DONUT_GRAPHICS_IMAGE_LDR_HPP
#define DONUT_GRAPHICS_IMAGE_LDR_HPP

#include <donut/graphics/Image.hpp>

#include <cstddef> // std::size_t, std::byte

namespace donut {
namespace graphics {

class ImageLDRView : public ImageView {
public:
	constexpr ImageLDRView() noexcept = default;

	constexpr ImageLDRView(const std::byte* pixels, std::size_t width, std::size_t height, std::size_t channelCount) noexcept
		: ImageView(pixels, width, height, channelCount) {}

	[[nodiscard]] constexpr const std::byte* getPixels() const noexcept {
		return static_cast<const std::byte*>(ImageView::getPixels());
	}
};

struct ImageLDROptions {
	int desiredChannelCount = 0;
	bool flipVertically = false;
};

class ImageLDR : public Image {
public:
	ImageLDR() noexcept = default;

	explicit ImageLDR(const char* filepath, const ImageLDROptions& options = {})
		: Image(filepath,
			  {
				  .desiredChannelCount = options.desiredChannelCount,
				  .highDynamicRange = false,
				  .flipVertically = options.flipVertically,
			  }) {}

	ImageLDR(const std::byte* pixels, std::size_t width, std::size_t height, std::size_t channelCount);

	operator ImageLDRView() const noexcept {
		return ImageLDRView{getPixels(), getWidth(), getHeight(), getChannelCount()};
	}

	[[nodiscard]] std::byte* getPixels() noexcept {
		return static_cast<std::byte*>(Image::getPixels());
	}

	[[nodiscard]] const std::byte* getPixels() const noexcept {
		return static_cast<const std::byte*>(Image::getPixels());
	}
};

} // namespace graphics
} // namespace donut

#endif
