#ifndef DONUT_GRAPHICS_IMAGE_HDR_HPP
#define DONUT_GRAPHICS_IMAGE_HDR_HPP

#include <donut/graphics/Image.hpp>

#include <cstddef> // std::size_t

namespace donut {
namespace graphics {

/**
 * Read-only non-owning view over a 2D image where each pixel comprises a
 * uniform number of 32-bit floating-point components, for high dynamic range.
 *
 * \sa ImageHDR
 */
class ImageHDRView : public ImageView {
public:
	/**
	 * Construct a view that does not reference an image.
	 */
	constexpr ImageHDRView() noexcept = default;

	/**
	 * Construct an image view over a contiguous 2D range of pixels where each
	 * pixel comprises a uniform number of 32-bit floating-point components.
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
	constexpr ImageHDRView(const float* pixels, std::size_t width, std::size_t height, std::size_t channelCount) noexcept
		: ImageView(pixels, width, height, channelCount) {}

	/**
	 * Get the pixel data referenced by this view.
	 *
	 * The pixel data is tightly packed and fully contiguous, and the component
	 * type is 32-bit floating-point, meaning the total size of the image in
	 * bytes is:
	 * ```
	 * width * height * channelCount * sizeof(float)
	 * ```
	 * and the size of a single pixel is `channelCount * sizeof(float)`. The
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
	 * \return a read-only non-owning pointer to the pixel data, or nullptr if
	 *         the view does not reference an image.
	 */
	[[nodiscard]] constexpr const float* getPixels() const noexcept {
		return static_cast<const float*>(ImageView::getPixels());
	}
};

/**
 * Options for saving a HDR image in Radiance HDR RGBE format.
 */
struct ImageHDRSaveHDROptions {
	/**
	 * Flip the saved image vertically.
	 */
	bool flipVertically = false;
};

/**
 * Options for loading an HDR image.
 */
struct ImageHDROptions {
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
 * In-memory representation of a 2D image with 32-bit floating-point pixel
 * components for high dynamic range.
 */
class ImageHDR : public Image {
public:
	/**
	 * Save an HDR image to a Radiance HDR RGBE file.
	 *
	 * \param image view over the image to save.
	 * \param filepath virtual filepath to save the image to, see File.
	 * \param options saving options, see ImageHDRSaveHDROptions.
	 *
	 * \throws File::Error on failure to create the file.
	 * \throws graphics::Error on failure to write the image to the file.
	 * \throws std::bad_alloc on allocation failure.
	 */
	static void saveHDR(const ImageHDRView& image, const char* filepath, const ImageHDRSaveHDROptions& options = {});

	/**
	 * Construct an empty image without a value.
	 */
	ImageHDR() noexcept = default;

	/**
	 * Load an HDR image from a virtual file.
	 *
	 * \param filepath virtual filepath of the image file to load, see File.
	 * \param options image options, see ImageHDROptions.
	 *
	 * \throws File::Error on failure to open the file.
	 * \throws graphics::Error on failure to load an image from the file.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note See Image::Image() for the supported file formats and limitations.
	 * \note If the loaded image is not high dynamic range, the image is
	 *       automatically converted from sRGB to linear color assuming a gamma
	 *       value of 2.2.
	 */
	explicit ImageHDR(const char* filepath, const ImageHDROptions& options = {})
		: Image(filepath,
			  {
				  .desiredChannelCount = options.desiredChannelCount,
				  .highDynamicRange = true,
				  .flipVertically = options.flipVertically,
			  }) {}

	/**
	 * Construct an image copied from a contiguous 2D range of pixels where each
	 * pixel comprises a uniform number of 32-bit floating-point components.
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
	 *          `width * height * channelCount` values of type float, the result
	 *          is undefined.
	 * \warning If the values of width, height and channelCount are such that
	 *          the expression `width * height * channelCount * sizeof(float)`
	 *          overflows, the result is undefined.
	 */
	ImageHDR(const float* pixels, std::size_t width, std::size_t height, std::size_t channelCount);

	/**
	 * Get a view over this image.
	 *
	 * \return if the image has a value, returns a read-only non-owning view
	 *         over it. Otherwise, returns a view that doesn't reference an
	 *         image.
	 */
	operator ImageHDRView() const noexcept {
		return ImageHDRView{getPixels(), getWidth(), getHeight(), getChannelCount()};
	}

	/**
	 * Get the pixel data stored in this image.
	 *
	 * The pixel data is tightly packed and fully contiguous, and the component
	 * type is 32-bit floating-point, meaning the total size of the image in
	 * bytes is:
	 * ```
	 * width * height * channelCount * sizeof(float)
	 * ```
	 * and the size of a single pixel is `channelCount * sizeof(float)`. The
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
	 * \return a non-owning pointer to the pixel data, or nullptr if the view
	 *         does not reference an image.
	 */
	[[nodiscard]] float* getPixels() noexcept {
		return static_cast<float*>(Image::getPixels());
	}

	/**
	 * Get the pixel data stored in this image.
	 *
	 * The pixel data is tightly packed and fully contiguous, and the component
	 * type is 32-bit floating-point, meaning the total size of the image in
	 * bytes is:
	 * ```
	 * width * height * channelCount * sizeof(float)
	 * ```
	 * and the size of a single pixel is `channelCount * sizeof(float)`. The
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
	 * \return a read-only non-owning pointer to the pixel data, or nullptr if
	 *         the view does not reference an image.
	 */
	[[nodiscard]] const float* getPixels() const noexcept {
		return static_cast<const float*>(Image::getPixels());
	}
};

} // namespace graphics
} // namespace donut

#endif
