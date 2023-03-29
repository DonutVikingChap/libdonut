#ifndef DONUT_GRAPHICS_IMAGE_HPP
#define DONUT_GRAPHICS_IMAGE_HPP

#include <donut/Resource.hpp>

#include <cassert>  // assert
#include <cstddef>  // std::size_t
#include <cstdint>  // std::uint32_t
#include <optional> // std::optional
#include <utility>  // std::move

namespace donut {
namespace graphics {

/**
 * Description of the number and meaning of the pixel component channels of an
 * image.
 */
enum class PixelFormat : std::uint32_t {
	R = 0x1903,    ///< Each pixel comprises 1 component: red. \hideinitializer
	RG = 0x8227,   ///< Each pixel comprises 2 components: red, green. \hideinitializer
	RGB = 0x1907,  ///< Each pixel comprises 3 components: red, green, blue. \hideinitializer
	RGBA = 0x1908, ///< Each pixel comprises 4 components: red, green, blue, alpha. \hideinitializer
};

/**
 * Description of the data type of the pixel components of an image.
 */
enum class PixelComponentType : std::uint32_t {
	U8 = 0x1401,  ///< Each pixel component is an 8-bit unsigned integer. \hideinitializer
	F16 = 0x140B, ///< Each pixel component is a 16-bit floating-point number. \hideinitializer
	F32 = 0x1406, ///< Each pixel component is a 32-bit floating-point number. \hideinitializer
};

/**
 * Read-only non-owning view over a 2D image.
 *
 * \sa Image
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
	 * \param width width of the image, in pixels. Must be 0 if pixels is
	 *        nullptr.
	 * \param height height of the image, in pixels. Must be 0 if pixels is
	 *        nullptr.
	 * \param pixelFormat pixel format of the image. Must be PixelFormat::R if
	 *        pixels is nullptr.
	 * \param pixelComponentType pixel component data type of the image. Must be
	 *        PixelComponentType::U8 if pixels is nullptr.
	 * \param pixels read-only non-owning pointer to the pixel data, or nullptr
	 *        to create a view that doesn't reference an image.
	 */
	constexpr ImageView(std::size_t width, std::size_t height, PixelFormat pixelFormat, PixelComponentType pixelComponentType, const void* pixels) noexcept
		: pixels(pixels)
		, width(width)
		, height(height)
		, pixelFormat(pixelFormat)
		, pixelComponentType(pixelComponentType) {
		assert(pixels || (width == 0 && height == 0 && pixelFormat == PixelFormat::R && pixelComponentType == PixelComponentType::U8));
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
	 * Get the width of the image referenced by this view.
	 *
	 * \return the width of the image, in pixels, or 0 if the view does not
	 *         reference an image.
	 *
	 * \sa getHeight()
	 * \sa getSizeInBytes()
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
	 * \sa getSizeInBytes()
	 */
	[[nodiscard]] constexpr std::size_t getHeight() const noexcept {
		return height;
	}

	/**
	 * Get the pixel format of the image referenced by this view.
	 *
	 * \return the pixel format, or PixelFormat::R if the view does not
	 *         reference an image.
	 *
	 * \sa getChannelCount()
	 */
	[[nodiscard]] constexpr PixelFormat getPixelFormat() const noexcept {
		return pixelFormat;
	}

	/**
	 * Get the pixel component type of the image referenced by this view.
	 *
	 * \return the pixel component type, or PixelComponentType::U8 if the view
	 *         does not reference an image.
	 *
	 * \sa getPixelComponentSize()
	 */
	[[nodiscard]] constexpr PixelComponentType getPixelComponentType() const noexcept {
		return pixelComponentType;
	}

	/**
	 * Get the pixel data referenced by this view.
	 *
	 * The pixel data is tightly packed and fully contiguous, so the total size
	 * of the image in bytes is:
	 * ```
	 * width * height * pixelSize
	 * ```.
	 *
	 * The size of a single pixel, `pixelSize`, is
	 * ```
	 * pixelFormatChannelCount * pixelComponentTypeSize
	 * ```
	 * where `pixelFormatChannelCount` is the number of component channels in
	 * the pixel format, and `pixelComponentTypeSize` is the size in bytes of a
	 * single pixel component.
	 *
	 * The pixels are stored in row-major order, starting at the bottom left of
	 * the image (unless the image is flipped).
	 *
	 * \return a read-only non-owning pointer to the pixel data, or nullptr if
	 *         the view does not reference an image.
	 *
	 * \sa getWidth()
	 * \sa getHeight()
	 * \sa getPixelFormat()
	 * \sa getPixelComponentType()
	 */
	[[nodiscard]] constexpr const void* getPixels() const noexcept {
		return pixels;
	}

	/**
	 * Get the number of component channels in the pixel format of the image
	 * referenced by this view.
	 *
	 * \return the number of channels, or 0 if the view does not reference an
	 *         image.
	 *
	 * \sa getPixelFormat()
	 * \sa getPixelStride()
	 */
	[[nodiscard]] constexpr std::size_t getChannelCount() const noexcept {
		if (pixels) {
			switch (pixelFormat) {
				case PixelFormat::R: return 1;
				case PixelFormat::RG: return 2;
				case PixelFormat::RGB: return 3;
				case PixelFormat::RGBA: return 4;
			}
		}
		return 0;
	}

	/**
	 * Get the size in bytes of a single component of a pixel in the image
	 * referenced by this view.
	 *
	 * \return the size of the pixel component type, or 0 if the view does not
	 *         reference an image.
	 *
	 * \sa getPixelComponentType()
	 * \sa getPixelStride()
	 */
	[[nodiscard]] constexpr std::size_t getPixelComponentSize() const noexcept {
		if (pixels) {
			switch (pixelComponentType) {
				case PixelComponentType::U8: return 1;
				case PixelComponentType::F16: return 2;
				case PixelComponentType::F32: return 4;
			}
		}
		return 0;
	}

	/**
	 * Get the stride in bytes of the pixels in the image referenced by this
	 * view.
	 *
	 * \return the number of bytes to advance to get from one pixel to the next,
	 *         or 0 if the view does not reference an image.
	 *
	 * \sa getChannelCount()
	 * \sa getPixelComponentSize()
	 * \sa getSizeInBytes()
	 */
	[[nodiscard]] constexpr std::size_t getPixelStride() const noexcept {
		return getChannelCount() * getPixelComponentSize();
	}

	/**
	 * Get the size in bytes of the image referenced by this view.
	 *
	 * \return the total size of the image, or 0 if the view does not reference
	 *         an image.
	 *
	 * \sa getWidth()
	 * \sa getHeight()
	 * \sa getPixelStride()
	 */
	[[nodiscard]] constexpr std::size_t getSizeInBytes() const noexcept {
		return getWidth() * getHeight() * getPixelStride();
	}

private:
	const void* pixels = nullptr;
	std::size_t width = 0;
	std::size_t height = 0;
	PixelFormat pixelFormat = PixelFormat::R;
	PixelComponentType pixelComponentType = PixelComponentType::U8;
};

/**
 * Options for saving an image in PNG format.
 */
struct ImageSavePNGOptions {
	/**
	 * PNG compression level.
	 *
	 * Use a higher value for a higher degree of compression and smaller file
	 * size at the cost of encoding/decoding performance. The compression is
	 * lossless.
	 */
	int compressionLevel = 8;

	/**
	 * Flip the saved image vertically.
	 */
	bool flipVertically = false;
};

/**
 * Options for saving an image in Windows Bitmap format.
 */
struct ImageSaveBMPOptions {
	/**
	 * Flip the saved image vertically.
	 */
	bool flipVertically = false;
};

/**
 * Options for saving an image in Truevision TARGA format.
 */
struct ImageSaveTGAOptions {
	/**
	 * Use run-length encoding to compress the image.
	 *
	 * This kind of compression works best for simple images with infrequent
	 * changes in color. The compression is lossless.
	 */
	bool useRleCompression = true;

	/**
	 * Flip the saved image vertically.
	 */
	bool flipVertically = false;
};

/**
 * Options for saving an image in JPEG format.
 */
struct ImageSaveJPGOptions {
	/**
	 * JPEG quality.
	 *
	 * Higher values yield better image quality but results in a larger file
	 * size. The compression is lossy.
	 */
	int quality = 90;

	/**
	 * Flip the saved image vertically.
	 */
	bool flipVertically = false;
};

/**
 * Options for saving an image in Radiance HDR RGBE format.
 */
struct ImageSaveHDROptions {
	/**
	 * Flip the saved image vertically.
	 */
	bool flipVertically = false;
};

/**
 * Options for loading an image.
 */
struct ImageOptions {
	/**
	 * If set, request the loaded image to be converted to this format.
	 */
	std::optional<PixelFormat> desiredFormat{};

	/**
	 * Load and store the image with high dynamic range.
	 *
	 * If set to true, the pixel component type will be PixelComponentType::F32.
	 * Otherwise, the component type is PixelComponentType::U8.
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
 * Container for a 2D image.
 *
 * \sa ImageView
 */
class Image {
public:
	/**
	 * Save an 8-bit-per-channel image to a PNG file.
	 *
	 * \param image view over the image to save.
	 * \param filepath virtual filepath to save the image to, see File.
	 * \param options saving options, see ImageSavePNGOptions.
	 *
	 * \note This function will fail if the image does not have pixel component
	 *       type PixelComponentType::U8.
	 *
	 * \throws File::Error on failure to create the file.
	 * \throws graphics::Error on failure to write the image to the file.
	 * \throws std::bad_alloc on allocation failure.
	 */
	static void savePNG(const ImageView& image, const char* filepath, const ImageSavePNGOptions& options = {});

	/**
	 * Save an 8-bit-per-channel image to a Windows Bitmap file.
	 *
	 * \param image view over the image to save.
	 * \param filepath virtual filepath to save the image to, see File.
	 * \param options saving options, see ImageSaveBMPOptions.
	 *
	 * \note This function will fail if the image does not have pixel component
	 *       type PixelComponentType::U8.
	 *
	 * \throws File::Error on failure to create the file.
	 * \throws graphics::Error on failure to write the image to the file.
	 * \throws std::bad_alloc on allocation failure.
	 */
	static void saveBMP(const ImageView& image, const char* filepath, const ImageSaveBMPOptions& options = {});

	/**
	 * Save an 8-bit-per-channel image to a Truevision TARGA file.
	 *
	 * \param image view over the image to save.
	 * \param filepath virtual filepath to save the image to, see File.
	 * \param options saving options, see ImageSaveTGAOptions.
	 *
	 * \note This function will fail if the image does not have pixel component
	 *       type PixelComponentType::U8.
	 *
	 * \throws File::Error on failure to create the file.
	 * \throws graphics::Error on failure to write the image to the file.
	 * \throws std::bad_alloc on allocation failure.
	 */
	static void saveTGA(const ImageView& image, const char* filepath, const ImageSaveTGAOptions& options = {});

	/**
	 * Save an 8-bit-per-channel image to a JPEG file.
	 *
	 * \param image view over the image to save.
	 * \param filepath virtual filepath to save the image to, see File.
	 * \param options saving options, see ImageSaveJPGOptions.
	 *
	 * \note This function will fail if the image does not have pixel component
	 *       type PixelComponentType::U8.
	 *
	 * \throws File::Error on failure to create the file.
	 * \throws graphics::Error on failure to write the image to the file.
	 * \throws std::bad_alloc on allocation failure.
	 */
	static void saveJPG(const ImageView& image, const char* filepath, const ImageSaveJPGOptions& options = {});

	/**
	 * Save a floating-point 32-bit-per-channel image to a Radiance HDR RGBE
	 * file.
	 *
	 * \param image view over the image to save.
	 * \param filepath virtual filepath to save the image to, see File.
	 * \param options saving options, see ImageSaveHDROptions.
	 *
	 * \note This function will fail if the image does not have pixel component
	 *       type PixelComponentType::F32.
	 *
	 * \throws File::Error on failure to create the file.
	 * \throws graphics::Error on failure to write the image to the file.
	 * \throws std::bad_alloc on allocation failure.
	 */
	static void saveHDR(const ImageView& image, const char* filepath, const ImageSaveHDROptions& options = {});

	/**
	 * Construct an empty image without a value.
	 */
	Image() noexcept = default;

	/**
	 * Construct an image copied from a contiguous 2D range of pixels.
	 *
	 * \param width width of the image, in pixels. Must be 0 if pixels is
	 *        nullptr.
	 * \param height height of the image, in pixels. Must be 0 if pixels is
	 *        nullptr.
	 * \param pixelFormat pixel format of the image. Must be PixelFormat::R if
	 *        pixels is nullptr.
	 * \param pixelComponentType pixel component data type of the image. Must be
	 *        PixelComponentType::U8 if pixels is nullptr.
	 * \param pixels read-only non-owning pointer to the pixel data to copy, or
	 *        nullptr to create an empty image without a value.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \warning If pixels is not nullptr and does not point to a readable
	 *          contiguous region of memory containing image data of the
	 *          specified size, format and type, the result is undefined.
	 */
	Image(std::size_t width, std::size_t height, PixelFormat pixelFormat, PixelComponentType pixelComponentType, const void* pixels);

	/**
	 * Construct an image copied from an image view.
	 *
	 * \param image read-only view over the image to copy.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 */
	explicit Image(const ImageView& image);

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
		return ImageView{getWidth(), getHeight(), getPixelFormat(), getPixelComponentType(), getPixels()};
	}

	/**
	 * Remove the value from this image and reset it to an empty image.
	 */
	void reset() noexcept {
		*this = Image{};
	}

	/**
	 * Get the width of the image.
	 *
	 * \return the width of the image, in pixels, or 0 if the image does not
	 *         have a value.
	 *
	 * \sa getHeight()
	 * \sa getSizeInBytes()
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
	 * \sa getSizeInBytes()
	 */
	[[nodiscard]] std::size_t getHeight() const noexcept {
		return height;
	}

	/**
	 * Get the pixel format of the image.
	 *
	 * \return the pixel format, or PixelFormat::R if the image does not have a
	 *         value.
	 *
	 * \sa getChannelCount()
	 */
	[[nodiscard]] PixelFormat getPixelFormat() const noexcept {
		return pixelFormat;
	}

	/**
	 * Get the pixel component type of the image.
	 *
	 * \return the pixel component type, or PixelComponentType::U8 if the image
	 *         does not have a value.
	 *
	 * \sa getPixelComponentSize()
	 */
	[[nodiscard]] PixelComponentType getPixelComponentType() const noexcept {
		return pixelComponentType;
	}

	/**
	 * Get the pixel data of this image.
	 *
	 * The pixel data is tightly packed and fully contiguous, and the pixels are
	 * stored in row-major order, starting at the bottom left of the image
	 * (unless the image is flipped).
	 *
	 * \return a non-owning pointer to the pixel data, or nullptr if the image
	 *         does not have a value.
	 *
	 * \sa getWidth()
	 * \sa getHeight()
	 * \sa getPixelFormat()
	 * \sa getPixelComponentType()
	 */
	[[nodiscard]] void* getPixels() noexcept {
		return pixels.get();
	}

	/**
	 * Get the pixel data of this image.
	 *
	 * The pixel data is tightly packed and fully contiguous, and the pixels are
	 * stored in row-major order, starting at the bottom left of the image
	 * (unless the image is flipped).
	 *
	 * \return a read-only non-owning pointer to the pixel data, or nullptr if
	 *         the image does not have a value.
	 *
	 * \sa getWidth()
	 * \sa getHeight()
	 * \sa getPixelFormat()
	 * \sa getPixelComponentType()
	 */
	[[nodiscard]] const void* getPixels() const noexcept {
		return pixels.get();
	}

	/**
	 * Get the number of component channels in the pixel format of this image.
	 *
	 * \return the number of channels, or 0 if the image does not have a value.
	 *
	 * \sa getPixelFormat()
	 * \sa getPixelStride()
	 */
	[[nodiscard]] std::size_t getChannelCount() const noexcept {
		return ImageView{*this}.getChannelCount();
	}

	/**
	 * Get the size in bytes of a single component of a pixel in this image.
	 *
	 * \return the size of the pixel component type, or 0 if the image does not
	 *         have a value.
	 *
	 * \sa getPixelComponentType()
	 * \sa getPixelStride()
	 */
	[[nodiscard]] std::size_t getPixelComponentSize() const noexcept {
		return ImageView{*this}.getPixelComponentSize();
	}

	/**
	 * Get the stride in bytes of the pixels in this image.
	 *
	 * \return the number of bytes to advance to get from one pixel to the next,
	 *         or 0 if the image does not have a value.
	 *
	 * \sa getChannelCount()
	 * \sa getPixelComponentSize()
	 * \sa getSizeInBytes()
	 */
	[[nodiscard]] std::size_t getPixelStride() const noexcept {
		return ImageView{*this}.getPixelStride();
	}

	/**
	 * Get the size in bytes of this image.
	 *
	 * \return the total size of the image, or 0 if the image does not have a
	 *         value.
	 *
	 * \sa getWidth()
	 * \sa getHeight()
	 * \sa getPixelStride()
	 */
	[[nodiscard]] std::size_t getSizeInBytes() const noexcept {
		return ImageView{*this}.getSizeInBytes();
	}

private:
	struct PixelsDeleter {
		void operator()(void* handle) const noexcept;
	};

	using Pixels = Resource<void*, PixelsDeleter, nullptr>;

	Pixels pixels{};
	std::size_t width = 0;
	std::size_t height = 0;
	PixelFormat pixelFormat = PixelFormat::R;
	PixelComponentType pixelComponentType = PixelComponentType::U8;
};

} // namespace graphics
} // namespace donut

#endif
