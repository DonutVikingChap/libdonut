#ifndef DONUT_GRAPHICS_IMAGE_HPP
#define DONUT_GRAPHICS_IMAGE_HPP

#include <donut/Resource.hpp>

#include <cstddef> // std::size_t
#include <utility> // std::move

namespace donut {
namespace graphics {

class ImageView {
public:
	constexpr ImageView() noexcept = default;

	constexpr ImageView(const void* pixels, std::size_t width, std::size_t height, std::size_t channelCount) noexcept
		: pixels(pixels)
		, width(width)
		, height(height)
		, channelCount(channelCount) {}

	explicit operator bool() const noexcept {
		return static_cast<bool>(pixels);
	}

	[[nodiscard]] constexpr const void* getPixels() const noexcept {
		return pixels;
	}

	[[nodiscard]] constexpr std::size_t getWidth() const noexcept {
		return width;
	}

	[[nodiscard]] constexpr std::size_t getHeight() const noexcept {
		return height;
	}

	[[nodiscard]] constexpr std::size_t getChannelCount() const noexcept {
		return channelCount;
	}

private:
	const void* pixels = nullptr;
	std::size_t width = 0;
	std::size_t height = 0;
	std::size_t channelCount = 0;
};

struct ImageOptions {
	int desiredChannelCount = 0;
	bool highDynamicRange = false;
	bool flipVertically = false;
};

struct ImageSavePngOptions {
	int compressionLevel = 8;
	bool flipVertically = false;
};

struct ImageSaveBmpOptions {
	bool flipVertically = false;
};

struct ImageSaveTgaOptions {
	bool useRleCompression = true;
	bool flipVertically = false;
};

struct ImageSaveJpgOptions {
	int quality = 90;
	bool flipVertically = false;
};

struct ImageSaveHdrOptions {
	bool flipVertically = false;
};

class Image {
public:
	static void savePng(const ImageView& image, const char* filepath, const ImageSavePngOptions& options = {});
	static void saveBmp(const ImageView& image, const char* filepath, const ImageSaveBmpOptions& options = {});
	static void saveTga(const ImageView& image, const char* filepath, const ImageSaveTgaOptions& options = {});
	static void saveJpg(const ImageView& image, const char* filepath, const ImageSaveJpgOptions& options = {});
	static void saveHdr(const ImageView& image, const char* filepath, const ImageSaveHdrOptions& options = {});

	Image() noexcept = default;

	explicit Image(const char* filepath, const ImageOptions& options = {});

	explicit operator bool() const noexcept {
		return static_cast<bool>(pixels);
	}

	operator ImageView() const noexcept {
		return ImageView{getPixels(), getWidth(), getHeight(), getChannelCount()};
	}

	void reset() noexcept {
		*this = Image{};
	}

	[[nodiscard]] void* getPixels() noexcept {
		return pixels.get();
	}

	[[nodiscard]] const void* getPixels() const noexcept {
		return pixels.get();
	}

	[[nodiscard]] std::size_t getWidth() const noexcept {
		return width;
	}

	[[nodiscard]] std::size_t getHeight() const noexcept {
		return height;
	}

	[[nodiscard]] std::size_t getChannelCount() const noexcept {
		return channelCount;
	}

protected:
	struct PixelsDeleter {
		void operator()(void* handle) const noexcept;
	};

	using Pixels = Resource<void*, PixelsDeleter, nullptr>;

	Image(Pixels pixels, std::size_t width, std::size_t height, std::size_t channelCount)
		: pixels(std::move(pixels))
		, width(width)
		, height(height)
		, channelCount(channelCount) {}

private:
	Pixels pixels{};
	std::size_t width = 0;
	std::size_t height = 0;
	std::size_t channelCount = 0;
};

} // namespace graphics
} // namespace donut

#endif
