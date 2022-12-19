#ifndef DONUT_GRAPHICS_IMAGE_LDR_HPP
#define DONUT_GRAPHICS_IMAGE_LDR_HPP

#include <donut/graphics/Image.hpp>

#include <cstddef> // std::size_t, std::byte

namespace donut {
namespace graphics {

/**
 * Optional read-only non-owning view over a 2D image where each pixel comprises
 * a uniform number of 8-bit unsigned integer components, for low dynamic range.
 *
 * \sa ImageLDR
 */
class ImageLDRView : public ImageView {
public:
	/**
	 * Construct a view that does not reference an image.
	 */
	constexpr ImageLDRView() noexcept = default;

	/**
	 * Construct an image view over a contiguous 2D range of pixels where each
	 * pixel comprises a uniform number of 8-bit unsigned integer components.
	 *
	 * \param pixels read-only non-owning pointer to the pixel data, or nullptr
	 *        to create a view that doesn't reference an image. The pixel data
	 *        must be formatted according to description in getPixels().
	 * \param width width of the image, in pixels. Must be 0 if pixels is
	 *        nullptr.
	 * \param height height of the image, in pixels. Must be 0 if pixels is
	 *        nullptr.
	 * \param channelCount number of components per pixel. Must be less than or
	 *        equal to 4, and must be 0 if pixels is nullptr.
	 */
	constexpr ImageLDRView(const std::byte* pixels, std::size_t width, std::size_t height, std::size_t channelCount) noexcept
		: ImageView(pixels, width, height, channelCount) {}

	/**
	 * Get the pixel data referenced by this view.
	 *
	 * The pixel data is tightly packed and fully contiguous, and the component
	 * type is 8-bit unsigned integer, meaning the total size of the image in
	 * bytes is:
	 * ```
	 * width * height * channelCount
	 * ```
	 * and the size of a single pixel is `channelCount`. The pixels are stored
	 * in row-major order starting at the top left of the image.
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
	 * \return a read-only non-owning pointer to the pixel data, or nullptr if
	 *         the view does not reference an image.
	 */
	[[nodiscard]] constexpr const std::byte* getPixels() const noexcept {
		return static_cast<const std::byte*>(ImageView::getPixels());
	}
};

/**
 * Options for saving an LDR image in PNG format.
 */
struct ImageLDRSavePNGOptions {
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
 * Options for saving an LDR image in Windows Bitmap format.
 */
struct ImageLDRSaveBMPOptions {
	/**
	 * Flip the saved image vertically.
	 */
	bool flipVertically = false;
};

/**
 * Options for saving an LDR image in Truevision TARGA format.
 */
struct ImageLDRSaveTGAOptions {
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
 * Options for saving an LDR image in JPEG format.
 */
struct ImageLDRSaveJPGOptions {
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
 * Options for loading an LDR image.
 */
struct ImageLDROptions {
	/**
	 * If non-zero, request the loaded image to be converted to this number of
	 * channels.
	 *
	 * \warning If set greater than 4, the result is undefined.
	 */
	std::size_t desiredChannelCount = 0;

	/**
	 * Flip the loaded image vertically.
	 */
	bool flipVertically = false;
};

/**
 * In-memory representation of a 2D image with 8-bit unsigned integer pixel
 * components for low dynamic range.
 */
class ImageLDR : public Image {
public:
	/**
	 * Save an LDR image to a PNG file.
	 *
	 * \param image view over the image to save.
	 * \param filepath virtual filepath to save the image to, see File.
	 * \param options saving options, see ImageLDRSavePNGOptions.
	 *
	 * \throws File::Error on failure to create the file.
	 * \throws graphics::Error on failure to write the image to the file.
	 * \throws std::bad_alloc on allocation failure.
	 */
	static void savePNG(const ImageLDRView& image, const char* filepath, const ImageLDRSavePNGOptions& options = {});

	/**
	 * Save an LDR image to a Windows Bitmap file.
	 *
	 * \param image view over the image to save.
	 * \param filepath virtual filepath to save the image to, see File.
	 * \param options saving options, see ImageLDRSaveBMPOptions.
	 *
	 * \throws File::Error on failure to create the file.
	 * \throws graphics::Error on failure to write the image to the file.
	 * \throws std::bad_alloc on allocation failure.
	 */
	static void saveBMP(const ImageLDRView& image, const char* filepath, const ImageLDRSaveBMPOptions& options = {});

	/**
	 * Save an LDR image to a Truevision TARGA file.
	 *
	 * \param image view over the image to save.
	 * \param filepath virtual filepath to save the image to, see File.
	 * \param options saving options, see ImageLDRSaveTGAOptions.
	 *
	 * \throws File::Error on failure to create the file.
	 * \throws graphics::Error on failure to write the image to the file.
	 * \throws std::bad_alloc on allocation failure.
	 */
	static void saveTGA(const ImageLDRView& image, const char* filepath, const ImageLDRSaveTGAOptions& options = {});

	/**
	 * Save an LDR image to a JPEG file.
	 *
	 * \param image view over the image to save.
	 * \param filepath virtual filepath to save the image to, see File.
	 * \param options saving options, see ImageLDRSaveJPGOptions.
	 *
	 * \throws File::Error on failure to create the file.
	 * \throws graphics::Error on failure to write the image to the file.
	 * \throws std::bad_alloc on allocation failure.
	 */
	static void saveJPG(const ImageLDRView& image, const char* filepath, const ImageLDRSaveJPGOptions& options = {});

	/**
	 * Construct an empty image without a value.
	 */
	ImageLDR() noexcept = default;

	/**
	 * Load an LDR image from a virtual file.
	 *
	 * \param filepath virtual filepath of the image file to load, see File.
	 * \param options image options, see ImageLDROptions.
	 *
	 * \throws File::Error on failure to open the file.
	 * \throws graphics::Error on failure to load an image from the file.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note See Image::Image() for the supported file formats and limitations.
	 * \note If the loaded image is actually high dynamic range, the image is
	 *       automatically gamma corrected from linear color to sRGB assuming a
	 *       gamma value of 2.2.
	 */
	explicit ImageLDR(const char* filepath, const ImageLDROptions& options = {})
		: Image(filepath,
			  {
				  .desiredChannelCount = options.desiredChannelCount,
				  .highDynamicRange = false,
				  .flipVertically = options.flipVertically,
			  }) {}

	/**
	 * Construct an image copied from a contiguous 2D range of pixels where each
	 * pixel comprises a uniform number of 8-bit unsigned integer components.
	 *
	 * \param pixels read-only non-owning pointer to the pixel data to copy, or
	 *        nullptr to create an empty image without a value.
	 * \param width width of the image, in pixels. Must be 0 if pixels is
	 *        nullptr.
	 * \param height height of the image, in pixels. Must be 0 if pixels is
	 *        nullptr.
	 * \param channelCount number of components per pixel. Must be 0 if pixels
	 *        is nullptr.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \warning If pixels is not nullptr and does not point to a readable
	 *          contiguous region of memory containing
	 *          `width * height * channelCount` bytes, the result is undefined.
	 * \warning If the values of width, height and channelCount are such that
	 *          the expression `width * height * channelCount` overflows, the
	 *          result is undefined.
	 */
	ImageLDR(const std::byte* pixels, std::size_t width, std::size_t height, std::size_t channelCount);

	/**
	 * Get a view over this image.
	 *
	 * \return if the image has a value, returns a read-only non-owning view
	 *         over it. Otherwise, returns a view that doesn't reference an
	 *         image.
	 */
	operator ImageLDRView() const noexcept {
		return ImageLDRView{getPixels(), getWidth(), getHeight(), getChannelCount()};
	}

	/**
	 * Get the pixel data stored in this image.
	 *
	 * The pixel data is tightly packed and fully contiguous, and the component
	 * type is 8-bit unsigned integer, meaning the total size of the image in
	 * bytes is:
	 * ```
	 * width * height * channelCount
	 * ```
	 * and the size of a single pixel is `channelCount`. The pixels are stored
	 * in row-major order starting at the top left of the image.
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
	 * \return a non-owning pointer to the pixel data, or nullptr if the view
	 *         does not reference an image.
	 */
	[[nodiscard]] std::byte* getPixels() noexcept {
		return static_cast<std::byte*>(Image::getPixels());
	}

	/**
	 * Get the pixel data stored in this image.
	 *
	 * The pixel data is tightly packed and fully contiguous, and the component
	 * type is 8-bit unsigned integer, meaning the total size of the image in
	 * bytes is:
	 * ```
	 * width * height * channelCount
	 * ```
	 * and the size of a single pixel is `channelCount`. The pixels are stored
	 * in row-major order starting at the top left of the image.
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
	 * \return a read-only non-owning pointer to the pixel data, or nullptr if
	 *         the view does not reference an image.
	 */
	[[nodiscard]] const std::byte* getPixels() const noexcept {
		return static_cast<const std::byte*>(Image::getPixels());
	}
};

} // namespace graphics
} // namespace donut

#endif
