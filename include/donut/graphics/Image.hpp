#ifndef DONUT_GRAPHICS_IMAGE_HPP
#define DONUT_GRAPHICS_IMAGE_HPP

#include <donut/Resource.hpp>

#include <cassert> // assert
#include <cstddef> // std::size_t
#include <utility> // std::move

namespace donut {
namespace graphics {

/**
 * Optional read-only non-owning view over a 2D image with any pixel format.
 *
 * \sa ImageLDRView
 * \sa ImageHDRView
 * \sa Image
 * \sa ImageLDR
 * \sa ImageHDR
 */
class ImageView {
public:
	/**
	 * Construct a view that does not reference an image.
	 */
	constexpr ImageView() noexcept = default;

	/**
	 * Construct an image view over arbitrary 2D pixel data.
	 *
	 * \param pixels read-only non-owning pointer to the pixel data, or nullptr
	 *        to create a view that doesn't reference an image.
	 * \param width width of the image, in pixels. Must be 0 if pixels is
	 *        nullptr.
	 * \param height height of the image, in pixels. Must be 0 if pixels is
	 *        nullptr.
	 * \param channelCount number of components per pixel. Must be 0 if pixels
	 *        is nullptr.
	 */
	constexpr ImageView(const void* pixels, std::size_t width, std::size_t height, std::size_t channelCount) noexcept
		: pixels(pixels)
		, width(width)
		, height(height)
		, channelCount(channelCount) {
		assert(pixels || (width == 0 && height == 0 && channelCount == 0));
	}

	/**
	 * Check if this view references an image.
	 *
	 * \return true if the view references an image, false otherwise.
	 */
	explicit operator bool() const noexcept {
		return static_cast<bool>(pixels);
	}

	/**
	 * Get the pixel data referenced by this view.
	 *
	 * \return an untyped read-only non-owning pointer to the pixel data, or
	 *         nullptr if the view does not reference an image.
	 *
	 * \note The size and stride of the pixel data cannot be determined from the
	 *       information stored in this view alone. The pixel format information
	 *       must be implied through context or communicated through a separate
	 *       channel to the user of this view. This is the purpose of the
	 *       derived types ImageLDRView and ImageHDRView, which imply an 8-bit
	 *       or floating-point component format, respectively.
	 */
	[[nodiscard]] constexpr const void* getPixels() const noexcept {
		return pixels;
	}

	/**
	 * Get the width of the image referenced by this view.
	 *
	 * \return the width of the image, in pixels, or 0 if the view does not
	 *         reference an image.
	 *
	 * \sa getHeight()
	 * \sa getChannelCount()
	 */
	[[nodiscard]] constexpr std::size_t getWidth() const noexcept {
		return width;
	}

	/**
	 * Get the height of the image referenced by this view.
	 *
	 * \return the height of the image, in pixels, or 0 if the view does not
	 *         reference an image.
	 *
	 * \sa getWidth()
	 * \sa getChannelCount()
	 */
	[[nodiscard]] constexpr std::size_t getHeight() const noexcept {
		return height;
	}

	/**
	 * Get the number of components per pixel of the image referenced by
	 * this view.
	 *
	 * \return the number of channels, or 0 if the view does not reference an
	 *         image.
	 *
	 * \sa getWidth()
	 * \sa getHeight()
	 */
	[[nodiscard]] constexpr std::size_t getChannelCount() const noexcept {
		return channelCount;
	}

private:
	const void* pixels = nullptr;
	std::size_t width = 0;
	std::size_t height = 0;
	std::size_t channelCount = 0;
};

/**
 * Options for loading an image.
 */
struct ImageOptions {
	/**
	 * If non-zero, request the loaded image to be converted to this number of
	 * channels.
	 *
	 * \warning If set greater than 4, the result is undefined.
	 */
	std::size_t desiredChannelCount = 0;

	/**
	 * Load and store the image with high dynamic range.
	 *
	 * If set to true, the pixel component type will be 32-bit floating-point.
	 * Otherwise, the component type is 8-bit unsigned integer.
	 *
	 * If the loaded image is high dynamic range and this option is set to
	 * false, or vice versa, the image is automatically gamma corrected from
	 * sRGB to linear color or converted from linear color to sRGB, assuming a
	 * gamma value of 2.2 in both cases.
	 */
	bool highDynamicRange = false;

	/**
	 * Flip the loaded image vertically.
	 */
	bool flipVertically = false;
};

/**
 * In-memory representation of a 2D image with any pixel format.
 *
 * \sa ImageLDR
 * \sa ImageHDR
 * \sa ImageView
 * \sa ImageLDRView
 * \sa ImageHDRView
 */
class Image {
public:
	/**
	 * Construct an empty image without a value.
	 */
	Image() noexcept = default;

	/**
	 * Load an image from a virtual file.
	 *
	 * The supported file formats are:
	 * - JPEG (.jpg/.jpeg)
	 * - PNG (.png)
	 * - Truevision TARGA (.tga)
	 * - Windows Bitmap (.bmp)
	 * - Photoshop Document (.psd)
	 * - GIF (.gif)
	 * - Radiance HDR RGBE (.hdr)
	 * - Softimage PIC (.pic)
	 * - PPM (.ppm)
	 * - PGM (.pgm)
	 *
	 * \param filepath virtual filepath of the image file to load, see File.
	 * \param options image options, see ImageOptions.
	 *
	 * \throws File::Error on failure to open the file.
	 * \throws graphics::Error on failure to load an image from the file.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note The file format is determined entirely from the file contents; the
	 *       filename extension is not taken into account.
	 * \note The component type of the image is determined by the
	 *       ImageOptions::highDynamicRange option.
	 * \note For JPEG files, 12 bits per component and arithmetic coding are not
	 *       supported.
	 * \note PNG files support 1, 2, 4, 8 and 16 bits per channel.
	 * \note For BMP files, 1 bit per component and run-length encoding are not
	 *       supported.
	 * \note PSD files support 8 and 16 bits per pixel.
	 * \note For PSD files, only composited view is supported, with no extra
	 *       channels.
	 * \note For GIF files, animation is not supported, and the reported number
	 *       of channels is always 4.
	 * \note For PPM and PGM files, only binary format is supported.
	 */
	explicit Image(const char* filepath, const ImageOptions& options = {});

	/**
	 * Check if this image has a value.
	 *
	 * \return true if the image has a value, false otherwise.
	 */
	explicit operator bool() const noexcept {
		return static_cast<bool>(pixels);
	}

	/**
	 * Get a view over this image.
	 *
	 * \return if the image has a value, returns a read-only non-owning view
	 *         over it. Otherwise, returns a view that doesn't reference an
	 *         image.
	 */
	operator ImageView() const noexcept {
		return ImageView{getPixels(), getWidth(), getHeight(), getChannelCount()};
	}

	/**
	 * Remove the value from this image and reset it to an empty image.
	 */
	void reset() noexcept {
		*this = Image{};
	}

	/**
	 * Get the pixel data stored in this image.
	 *
	 * The pixel data is tightly packed and fully contiguous, but the format of
	 * the pixels depends on the loaded file and cannot be determined from the
	 * information stored in this object alone. The pixel format information
	 * must be implied through context or communicated through a separate
	 * channel to the user of this image. This is the purpose of the derived
	 * types ImageLDR and ImageHDR, which imply an 8-bit or floating-point
	 * component format, respectively. Assuming the component type is known, the
	 * total size of the image in bytes is:
	 * ```
	 * width * height * channelCount * componentSize
	 * ```
	 * and the size of a single pixel is `channelCount * componentSize`. The
	 * pixels are stored in row-major order starting at the top left of the
	 * image.
	 *
	 * Depending on the channel count, the pixel components have the following
	 * meanings:
	 *
	 * | channels | 0     | 1     | 2     | 3     |
	 * | -------- | ----- | ----- | ----- | ----- |
	 * |        1 | Gray  |       |       |       |
	 * |        2 | Gray  | Alpha |       |       |
	 * |        3 | Red   | Green | Blue  |       |
	 * |        4 | Red   | Green | Blue  | Alpha |
	 *
	 * \return an untyped non-owning pointer to the pixel data, or nullptr if
	 *         the image does not have a value.
	 */
	[[nodiscard]] void* getPixels() noexcept {
		return pixels.get();
	}

	/**
	 * Get the pixel data stored in this image.
	 *
	 * The pixel data is tightly packed and fully contiguous, but the format of
	 * the pixels depends on the loaded file and cannot be determined from the
	 * information stored in this object alone. The pixel format information
	 * must be implied through context or communicated through a separate
	 * channel to the user of this image. This is the purpose of the derived
	 * types ImageLDR and ImageHDR, which imply an 8-bit or floating-point
	 * component format, respectively. Assuming the component type is known, the
	 * total size of the image in bytes is:
	 * ```
	 * width * height * channelCount * componentSize
	 * ```
	 * and the size of a single pixel is `channelCount * componentSize`. The
	 * pixels are stored in row-major order starting at the top left of the
	 * image.
	 *
	 * Depending on the channel count, the pixel components have the following
	 * meanings:
	 *
	 * | channels | 0     | 1     | 2     | 3     |
	 * | -------- | ----- | ----- | ----- | ----- |
	 * |        1 | Gray  |       |       |       |
	 * |        2 | Gray  | Alpha |       |       |
	 * |        3 | Red   | Green | Blue  |       |
	 * |        4 | Red   | Green | Blue  | Alpha |
	 *
	 * \return an untyped read-only non-owning pointer to the pixel data, or
	 *         nullptr if the image does not have a value.
	 */
	[[nodiscard]] const void* getPixels() const noexcept {
		return pixels.get();
	}

	/**
	 * Get the width of the image.
	 *
	 * \return the width of the image, in pixels, or 0 if the image does not
	 *         have a value.
	 *
	 * \sa getHeight()
	 * \sa getChannelCount()
	 */
	[[nodiscard]] std::size_t getWidth() const noexcept {
		return width;
	}

	/**
	 * Get the height of the image.
	 *
	 * \return the height of the image, in pixels, or 0 if the image does not
	 *         have a value.
	 *
	 * \sa getWidth()
	 * \sa getChannelCount()
	 */
	[[nodiscard]] std::size_t getHeight() const noexcept {
		return height;
	}

	/**
	 * Get the number of components per pixel of this image.
	 *
	 * \return the number of channels, or 0 if the image does not have a value.
	 *
	 * \note The maximum number of channels is 4.
	 * \note The meaning of the pixel components is documented in getPixels().
	 *
	 * \sa getWidth()
	 * \sa getHeight()
	 */
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
