#ifndef DONUT_GRAPHICS_TEXTURE_HPP
#define DONUT_GRAPHICS_TEXTURE_HPP

#include <donut/Color.hpp>
#include <donut/Resource.hpp>
#include <donut/graphics/Handle.hpp>

#include <cstddef>     // std::size_t
#include <cstdint>     // std::int32_t, std::uint32_t
#include <glm/glm.hpp> // glm::...
#include <optional>    // std::optional

namespace donut {
namespace graphics {

class Renderer;     // Forward declaration, to avoid a circular include of Renderer.hpp.
class ImageLDRView; // Forward declaration, to avoid including ImageLDR.hpp.
class ImageHDRView; // Forward declaration, to avoid including ImageHDR.hpp.

/**
 * Description of the number and meaning of the pixel component channels of an
 * image when passed as an input to a Texture.
 */
enum class TextureFormat : std::uint32_t {
	NONE = 0,      ///< Invalid format. \hideinitializer
	R = 0x1903,    ///< Each pixel comprises 1 component: red. \hideinitializer
	RG = 0x8227,   ///< Each pixel comprises 2 components: red, green. \hideinitializer
	RGB = 0x1907,  ///< Each pixel comprises 3 components: red, green, blue. \hideinitializer
	RGBA = 0x1908, ///< Each pixel comprises 4 components: red, green, blue, alpha. \hideinitializer
};

/**
 * Description of the internal texel format of a Texture, including the number
 * of component channels, their meaning and their data type.
 */
enum class TextureInternalFormat : std::int32_t {
	NONE = 0,         ///< Invalid format. \hideinitializer
	R8 = 0x8229,      ///< Each texel comprises 1 8-bit unsigned integer component: red. \hideinitializer
	RG8 = 0x822B,     ///< Each texel comprises 2 8-bit unsigned integer components: red, green. \hideinitializer
	RGB8 = 0x8051,    ///< Each texel comprises 3 8-bit unsigned integer components: red, green, blue. \hideinitializer
	RGBA8 = 0x8058,   ///< Each texel comprises 4 8-bit unsigned integer components: red, green, blue, alpha. \hideinitializer
	R16F = 0x822D,    ///< Each texel comprises 1 16-bit floating-point component: red. \hideinitializer
	RG16F = 0x822F,   ///< Each texel comprises 2 16-bit floating-point components: red, green. \hideinitializer
	RGB16F = 0x881B,  ///< Each texel comprises 3 16-bit floating-point components: red, green, blue. \hideinitializer
	RGBA16F = 0x881A, ///< Each texel comprises 4 16-bit floating-point components: red, green, blue, alpha. \hideinitializer
	R32F = 0x822E,    ///< Each texel comprises 1 32-bit floating-point component: red. \hideinitializer
	RG32F = 0x8230,   ///< Each texel comprises 2 32-bit floating-point components: red, green. \hideinitializer
	RGB32F = 0x8815,  ///< Each texel comprises 3 32-bit floating-point components: red, green, blue. \hideinitializer
	RGBA32F = 0x8814, ///< Each texel comprises 4 32-bit floating-point components: red, green, blue, alpha. \hideinitializer
};

/**
 * Description of the data type of the pixel components of an image when passed
 * as an input to a Texture.
 */
enum class TextureComponentType : std::uint32_t {
	U8 = 0x1401,  ///< Each pixel component is an 8-bit unsigned integer. \hideinitializer
	F16 = 0x140B, ///< Each pixel component is a 16-bit floating-point number. \hideinitializer
	F32 = 0x1406, ///< Each pixel component is a 32-bit floating-point number. \hideinitializer
};

/**
 * Configuration options for a Texture and its assocated sampler.
 */
struct TextureOptions {
	/**
	 * Treat the texture as repeating when sampling it outside of the 0-1
	 * texture coordinate range.
	 *
	 * Disable to clamp to the edge of the texture instead.
	 */
	bool repeat = true;

	/**
	 * Use bilinear filtering rather than nearest-neighbor interpolation when
	 * sampling the texture at coordinates that do not correspond exactly to one
	 * texel.
	 *
	 * This results in a smoother appearance compared to the regular blocky
	 * nearest-neighbor filtering.
	 */
	bool useLinearFiltering = true;

	/**
	 * Generate a mipmap of different levels of detail for the texture and use
	 * it when sampling the texture at a smaller-than-1:1 scale.
	 *
	 * This results in fewer aliasing artifacts when rendering downscaled
	 * textures, such as those on distant 3D objects. It can also improve
	 * rendering performance slightly in those cases, at the cost of some extra
	 * texture memory.
	 */
	bool useMipmap = true;
};

/**
 * Storage for multidimensional data, such as 2D images, on the GPU, combined
 * with a sampler configuration that defines how to render the stored data.
 */
class Texture {
public:
	/**
	 * Pointer to the statically allocated storage for the built-in white 2D
	 * texture.
	 *
	 * The internal texel format is TextureInternalFormat::RGBA8.
	 * The color space is SRGB.
	 * The size is 1x1 texels.
	 *
	 * \warning This pointer must not be dereferenced in application code. It is
	 *          not guaranteed that the underlying texture will be present at
	 *          all times.
	 */
	static const Texture* const whiteR8G8B8A8Srgb1x1;

	/**
	 * Pointer to the statically allocated storage for the built-in mid-gray 2D
	 * texture.
	 *
	 * The internal texel format is TextureInternalFormat::RGBA8.
	 * The color space is linear.
	 * The size is 1x1 texels.
	 *
	 * \warning This pointer must not be dereferenced in application code. It is
	 *          not guaranteed that the underlying texture will be present at
	 *          all times.
	 */
	static const Texture* const grayR8G8B8A8Unorm1x1;

	/**
	 * Pointer to the statically allocated storage for the built-in normal-map
	 * 2D texture.
	 *
	 * The internal texel format is TextureInternalFormat::RGB8.
	 * The color space is linear.
	 * The size is 1x1 texels.
	 *
	 * \warning This pointer must not be dereferenced in application code. It is
	 *          not guaranteed that the underlying texture will be present at
	 *          all times.
	 */
	static const Texture* const normalR8G8B8Unorm1x1;

	/**
	 * Get the number of pixel component channels defined by a texture format.
	 *
	 * \param format the format to get the number of channels of.
	 *
	 * \return the number of channels in the given format.
	 */
	[[nodiscard]] static std::size_t getChannelCount(TextureFormat format) noexcept;

	/**
	 * Get the number of texel component channels defined by an internal texture
	 * format.
	 *
	 * \param format the format to get the number of channels of.
	 *
	 * \return the number of channels in the given format.
	 */
	[[nodiscard]] static std::size_t getInternalChannelCount(TextureInternalFormat internalFormat) noexcept;

	/**
	 * Get a description of the pixel component data type that corresponds to
	 * the texel component data type of an internal texture format.
	 *
	 * \param format the format to get the component data type of.
	 *
	 * \return the pixel data type that corresponds to the given format.
	 */
	[[nodiscard]] static TextureComponentType getInternalComponentType(TextureInternalFormat internalFormat) noexcept;

	/**
	 * Get an appropriate texture format where the number of pixel components
	 * corresponds to a desired number of channels.
	 *
	 * \param channelCount the desired number of pixel component channels.
	 *
	 * \return a texture format with the given number of channels, if one
	 *         exists.
	 *
	 * \throws graphics::Error if no appropriate texture format exists that
	 *         satisfies the constraint.
	 */
	[[nodiscard]] static TextureFormat getPixelFormat(std::size_t channelCount);

	/**
	 * Get an appropriate internal texture format where the number of texel
	 * components corresponds to a desired number of channels and the data type
	 * is 8-bit unsigned integer.
	 *
	 * \param channelCount the desired number of texel component channels.
	 *
	 * \return an internal texture format with 8-bit unsigned integer data type
	 *         and the given number of channels, if one exists.
	 *
	 * \throws graphics::Error if no appropriate internal texture format exists
	 *         that satisfies the constraints.
	 */
	[[nodiscard]] static TextureInternalFormat getTexelFormatU8(std::size_t channelCount);

	/**
	 * Get an appropriate internal texture format where the number of texel
	 * components corresponds to a desired number of channels and the data type
	 * is 16-bit floating-point.
	 *
	 * \param channelCount the desired number of texel component channels.
	 *
	 * \return an internal texture format with 16-bit floating-point data type
	 *         and the given number of channels, if one exists.
	 *
	 * \throws graphics::Error if no appropriate internal texture format exists
	 *         that satisfies the constraints.
	 */
	[[nodiscard]] static TextureInternalFormat getTexelFormatF16(std::size_t channelCount);

	/**
	 * Get an appropriate internal texture format where the number of texel
	 * components corresponds to a desired number of channels and the data type
	 * is 32-bit floating-point.
	 *
	 * \param channelCount the desired number of texel component channels.
	 *
	 * \return an internal texture format with 32-bit floating-point data type
	 *         and the given number of channels, if one exists.
	 *
	 * \throws graphics::Error if no appropriate internal texture format exists
	 *         that satisfies the constraints.
	 */
	[[nodiscard]] static TextureInternalFormat getTexelFormatF32(std::size_t channelCount);

	/**
	 * Construct an empty texture without a value.
	 */
	Texture() noexcept = default;

	/**
	 * Create a new texture object and allocate GPU memory for storing 2D image
	 * data.
	 *
	 * \param internalFormat internal texel format of the new texture.
	 * \param width width of the 2D image data to allocate, in texels.
	 * \param height height of the 2D image data to allocate, in texels.
	 * \param format pixel format of the input image.
	 * \param type pixel component data type of the input image.
	 * \param pixels non-owning read-only pointer to the pixel data of the input
	 *        image to copy into the new texture data storage, or nullptr to
	 *        leave the data uninitialized.
	 * \param options texture/sampler options, see TextureOptions.
	 *
	 * \throws graphics::Error on failure to create the texture object.
	 * \throws std::bad_alloc on allocation failure. Note: this pertains only to
	 *         CPU memory allocations. Failure to allocate GPU memory for the
	 *         texture data might not be reported directly.
	 *
	 * \warning If not nullptr, the pixel data pointed to by the pixels
	 *          parameter must be of the shape and format described by the
	 *          width, height, format and type parameters. Otherwise, the
	 *          behavior is undefined.
	 *
	 * \note For uninitialized data, consider using the Texture(TextureInternalFormat, std::size_t, std::size_t, const TextureOptions&)
	 *       overload instead of passing nullptr to the pixels parameter.
	 */
	Texture(TextureInternalFormat internalFormat, std::size_t width, std::size_t height, TextureFormat format, TextureComponentType type, const void* pixels,
		const TextureOptions& options = {});

	/**
	 * Create a new texture object and allocate GPU memory for storing an array
	 * of layers of 2D image data.
	 *
	 * \param internalFormat internal texel format of the new texture.
	 * \param width width of the 2D image data to allocate, in texels.
	 * \param height height of the 2D image data to allocate, in texels.
	 * \param depth number of 2D image layers to allocate for the array.
	 * \param format pixel format of the input image array.
	 * \param type pixel component data type of the input image array.
	 * \param pixels non-owning read-only pointer to the pixel data of the input
	 *        image array to copy into the new texture data storage, or nullptr
	 *        to leave the data uninitialized.
	 * \param options texture/sampler options, see TextureOptions.
	 *
	 * \throws graphics::Error on failure to create the texture object.
	 * \throws std::bad_alloc on allocation failure. Note: this pertains only to
	 *         CPU memory allocations. Failure to allocate GPU memory for the
	 *         texture data might not be reported directly.
	 *
	 * \warning If not nullptr, the pixel data array pointed to by the pixels
	 *          parameter must be of the shape and format described by the
	 *          width, height, depth, format and type parameters. Otherwise, the
	 *          behavior is undefined.
	 *
	 * \note For uninitialized data, consider using the Texture(TextureInternalFormat, std::size_t, std::size_t, std::size_t, const TextureOptions&)
	 *       overload instead of passing nullptr to the pixels parameter.
	 */
	Texture(TextureInternalFormat internalFormat, std::size_t width, std::size_t height, std::size_t depth, TextureFormat format, TextureComponentType type, const void* pixels,
		const TextureOptions& options = {});

	/**
	 * Create a new texture object and allocate uninitialized GPU memory for
	 * storing 2D image data.
	 *
	 * \param internalFormat internal texel format of the new texture.
	 * \param width width of the 2D image data to allocate, in texels.
	 * \param height height of the 2D image data to allocate, in texels.
	 * \param options texture/sampler options, see TextureOptions.
	 *
	 * \throws graphics::Error on failure to create the texture object.
	 * \throws std::bad_alloc on allocation failure. Note: this pertains only to
	 *         CPU memory allocations. Failure to allocate GPU memory for the
	 *         texture data might not be reported directly.
	 */
	Texture(TextureInternalFormat internalFormat, std::size_t width, std::size_t height, const TextureOptions& options = {});

	/**
	 * Create a new texture object and allocate uninitialized GPU memory for
	 * storing an array of layers of 2D image data.
	 *
	 * \param internalFormat internal texel format of the new texture.
	 * \param width width of the 2D image data to allocate, in texels.
	 * \param height height of the 2D image data to allocate, in texels.
	 * \param depth number of 2D image layers to allocate for the array.
	 * \param options texture/sampler options, see TextureOptions.
	 *
	 * \throws graphics::Error on failure to create the texture object.
	 * \throws std::bad_alloc on allocation failure. Note: this pertains only to
	 *         CPU memory allocations. Failure to allocate GPU memory for the
	 *         texture data might not be reported directly.
	 */
	Texture(TextureInternalFormat internalFormat, std::size_t width, std::size_t height, std::size_t depth, const TextureOptions& options = {});

	/**
	 * Create a new texture object and allocate GPU memory for storing 2D image
	 * data loaded from an LDR image.
	 *
	 * \param image non-owning read-only view over the image to copy into the
	 *        new texture data storage. The allocated storage will be sized to
	 *        fit the image.
	 * \param options texture/sampler options, see TextureOptions.
	 *
	 * \throws graphics::Error on failure to create the texture object, or on
	 *         failure to choose an appropriate internal texel format for the
	 *         given image.
	 * \throws std::bad_alloc on allocation failure. Note: this pertains only to
	 *         CPU memory allocations. Failure to allocate GPU memory for the
	 *         texture data might not be reported directly.
	 *
	 * \note A suitable 8-bit unsigned integer internal texel format is chosen
	 *       automatically based on the number of pixel component channels in
	 *       the image. To choose the internal format manually, use the Texture(TextureInternalFormat, std::size_t, std::size_t, TextureFormat, TextureComponentType, const void*, const TextureOptions&)
	 *       overload instead.
	 */
	Texture(const ImageLDRView& image, const TextureOptions& options = {});

	/**
	 * Create a new texture object and allocate GPU memory for storing 2D image
	 * data loaded from an HDR image.
	 *
	 * \param image non-owning read-only view over the image to copy into the
	 *        new texture data storage. The allocated storage will be sized to
	 *        fit the image.
	 * \param options texture/sampler options, see TextureOptions.
	 *
	 * \throws graphics::Error on failure to create the texture object, or on
	 *         failure to choose an appropriate internal texel format for the
	 *         given image.
	 * \throws std::bad_alloc on allocation failure. Note: this pertains only to
	 *         CPU memory allocations. Failure to allocate GPU memory for the
	 *         texture data might not be reported directly.
	 *
	 * \note A suitable 16-bit floating-point internal texel format is chosen
	 *       automatically based on the number of pixel component channels in
	 *       the image. To choose the internal format manually, use the Texture(TextureInternalFormat, std::size_t, std::size_t, TextureFormat, TextureComponentType, const void*, const TextureOptions&)
	 *       overload instead.
	 */
	Texture(const ImageHDRView& image, const TextureOptions& options = {});

	/**
	 * Check if the texture has a value.
	 *
	 * \return true if the texture has a value, false otherwise.
	 */
	explicit operator bool() const noexcept {
		return static_cast<bool>(texture);
	}

	/**
	 * Apply a new configuration of texture/sampler options to the 2D texture.
	 *
	 * \param newOptions the new options to apply.
	 *
	 * \warning This function must only be called on textures that are set up to
	 *          store 2D image data. Otherwise, the behavior is undefined.
	 */
	void setOptions2D(const TextureOptions& newOptions);

	/**
	 * Apply a new configuration of texture/sampler options to the 2D array
	 * texture.
	 *
	 * \param newOptions the new options to apply.
	 *
	 * \warning This function must only be called on textures that are set up to
	 *          store an array of layers of 2D image data. Otherwise, the
	 *          behavior is undefined.
	 */
	void setOptions2DArray(const TextureOptions& newOptions);

	/**
	 * Copy 2D image data into the 2D texture at a specific position.
	 *
	 * \param width width of the 2D image data to copy, in pixels.
	 * \param height height of the 2D image data to copy, in pixels.
	 * \param format pixel format of the input image.
	 * \param type pixel component data type of the input image.
	 * \param pixels non-owning read-only pointer to the pixel data of the input
	 *        image to copy into the existing texture data storage. Must not be
	 *        nullptr.
	 * \param x the horizontal offset, in texels, from the left edge of the
	 *        texture at which to paste the image, where the left edge of the
	 *        pasted image will begin.
	 * \param y the vertical offset, in texels, from the bottom edge of the
	 *        texture at which to paste the image, where the bottom edge of the
	 *        pasted image will begin.
	 *
	 * \warning This function must only be called on textures that are set up to
	 *          store 2D image data. Otherwise, the behavior is undefined.
	 * \warning The pixel data pointed to by the pixels parameter must be of the
	 *          shape and format described by the width, height, format and type
	 *          parameters. Otherwise, the behavior is undefined.
	 * \warning Enough space must be allocated in the texture for the full image
	 *          to fit at the given position. Otherwise, the behavior is
	 *          undefined.
	 */
	void pasteImage2D(std::size_t width, std::size_t height, TextureFormat format, TextureComponentType type, const void* pixels, std::size_t x, std::size_t y);

	/**
	 * Copy an LDR 2D image into the 2D texture at a specific position.
	 *
	 * \param image non-owning read-only view over the image to copy into the
	 *        existing texture data storage.
	 * \param x the horizontal offset, in texels, from the left edge of the
	 *        texture at which to paste the image, where the left edge of the
	 *        pasted image will begin.
	 * \param y the vertical offset, in texels, from the bottom edge of the
	 *        texture at which to paste the image, where the bottom edge of the
	 *        pasted image will begin.
	 *
	 * \warning This function must only be called on textures that are set up to
	 *          store 2D image data. Otherwise, the behavior is undefined.
	 * \warning Enough space must be allocated in the texture for the full image
	 *          to fit at the given position. Otherwise, the behavior is
	 *          undefined.
	 */
	void pasteImage2D(const ImageLDRView& image, std::size_t x, std::size_t y);

	/**
	 * Copy an HDR 2D image into the 2D texture at a specific position.
	 *
	 * \param image non-owning read-only view over the image to copy into the
	 *        existing texture data storage.
	 * \param x the horizontal offset, in texels, from the left edge of the
	 *        texture at which to paste the image, where the left edge of the
	 *        pasted image will begin.
	 * \param y the vertical offset, in texels, from the bottom edge of the
	 *        texture at which to paste the image, where the bottom edge of the
	 *        pasted image will begin.
	 *
	 * \warning This function must only be called on textures that are set up to
	 *          store 2D image data. Otherwise, the behavior is undefined.
	 * \warning Enough space must be allocated in the texture for the full image
	 *          to fit at the given position. Otherwise, the behavior is
	 *          undefined.
	 */
	void pasteImage2D(const ImageHDRView& image, std::size_t x, std::size_t y);

	/**
	 * Copy an array of layers of 2D image data into the 2D array texture at a
	 * specific position.
	 *
	 * \param width width of the 2D image data to copy, in pixels.
	 * \param height height of the 2D image data to copy, in pixels.
	 * \param depth number of 2D image layers in the array to copy.
	 * \param format pixel format of the input image.
	 * \param type pixel component data type of the input image.
	 * \param pixels non-owning read-only pointer to the pixel data of the input
	 *        image array to copy into the existing texture data storage. Must
	 *        not be nullptr.
	 * \param x the horizontal offset, in texels, from the left edge of the
	 *        texture at which to paste the image array, where the left edge of
	 *        the pasted image array will begin.
	 * \param y the vertical offset, in texels, from the bottom edge of the
	 *        texture at which to paste the image array, where the bottom edge
	 *        of the pasted image array will begin.
	 * \param z the depth offset, in texels, from the first layer of the
	 *        texture at which to paste the image array, where the first pasted
	 *        image will begin.
	 *
	 * \warning This function must only be called on textures that are set up to
	 *          store arrays of layers of 2D image data. Otherwise, the behavior
	 *          is undefined.
	 * \warning The pixel data pointed to by the pixels parameter must be of the
	 *          shape and format described by the width, height, depth, format
	 *          and type parameters. Otherwise, the behavior is undefined.
	 * \warning Enough space must be allocated in the texture for the full image
	 *          array to fit at the given position. Otherwise, the behavior is
	 *          undefined.
	 */
	void pasteImage2DArray(
		std::size_t width, std::size_t height, std::size_t depth, TextureFormat format, TextureComponentType type, const void* pixels, std::size_t x, std::size_t y, std::size_t z);

	/**
	 * Copy an LDR 2D image into the 2D array texture at a specific position.
	 *
	 * \param image non-owning read-only view over the image to copy into the
	 *        existing texture data storage.
	 * \param x the horizontal offset, in texels, from the left edge of the
	 *        texture at which to paste the image, where the left edge of the
	 *        pasted image will begin.
	 * \param y the vertical offset, in texels, from the bottom edge of the
	 *        texture at which to paste the image, where the bottom edge of the
	 *        pasted image will begin.
	 * \param z the depth offset, in texels, from the first layer of the
	 *        texture at which to paste the image.
	 *
	 * \warning This function must only be called on textures that are set up to
	 *          store arrays of layers of 2D image data. Otherwise, the behavior
	 *          is undefined.
	 * \warning Enough space must be allocated in the texture for the full image
	 *          to fit at the given position. Otherwise, the behavior is
	 *          undefined.
	 */
	void pasteImage2DArray(const ImageLDRView& image, std::size_t x, std::size_t y, std::size_t z);

	/**
	 * Copy an HDR 2D image into the 2D array texture at a specific position.
	 *
	 * \param image non-owning read-only view over the image to copy into the
	 *        existing texture data storage.
	 * \param x the horizontal offset, in texels, from the left edge of the
	 *        texture at which to paste the image, where the left edge of the
	 *        pasted image will begin.
	 * \param y the vertical offset, in texels, from the bottom edge of the
	 *        texture at which to paste the image, where the bottom edge of the
	 *        pasted image will begin.
	 * \param z the depth offset, in texels, from the first layer of the
	 *        texture at which to paste the image.
	 *
	 * \warning This function must only be called on textures that are set up to
	 *          store arrays of layers of 2D image data. Otherwise, the behavior
	 *          is undefined.
	 * \warning Enough space must be allocated in the texture for the full image
	 *          to fit at the given position. Otherwise, the behavior is
	 *          undefined.
	 */
	void pasteImage2DArray(const ImageHDRView& image, std::size_t x, std::size_t y, std::size_t z);

	/**
	 * Fill the entire allocated 2D texture data with pixels of the given color.
	 *
	 * \param renderer renderer to use for filling the texture.
	 * \param color color to fill the texture with.
	 *
	 * \warning This function must only be called on textures that are set up to
	 *          store 2D image data. Otherwise, the behavior is undefined.
	 * \warning The internal texture format must be framebuffer-compatible,
	 *          otherwise the behavior is undefined.
	 */
	void fill2D(Renderer& renderer, Color color);

	/**
	 * Expand the allocated 2D texture data by allocating larger texture storage
	 * and copying the old texture image onto the bottom left corner of the new
	 * texture.
	 *
	 * \param renderer renderer to use for copying the texture data.
	 * \param newWidth new width of the 2D image data to allocate, in texels.
	 *        Must be greater than or equal to the old width.
	 * \param newHeight new height of the 2D image data to allocate, in texels.
	 *        Must be greater than or equal to the old height.
	 * \param backgroundColor if set, the new texture data is cleared to this
	 *        color before copying the old data onto it.
	 *
	 * \warning This function must only be called on textures that are set up to
	 *          store 2D image data. Otherwise, the behavior is undefined.
	 * \warning The internal texture format must be framebuffer-compatible,
	 *          otherwise the behavior is undefined.
	 *
	 * \throws std::bad_alloc on allocation failure. Note: this pertains only to
	 *         CPU memory allocations. Failure to allocate GPU memory for the
	 *         texture data might not be reported directly.
	 *
	 * \sa copy2D()
	 */
	void grow2D(Renderer& renderer, std::size_t newWidth, std::size_t newHeight, std::optional<Color> backgroundColor = {});

	/**
	 * Create a new texture object and allocate GPU memory onto which the 2D
	 * image data of this 2D texture is copied.
	 *
	 * \param renderer renderer to use for copying the texture data.
	 *
	 * \return the new copied texture.
	 *
	 * \warning This function must only be called on textures that are set up to
	 *          store 2D image data. Otherwise, the behavior is undefined.
	 * \warning The internal texture format must be framebuffer-compatible,
	 *          otherwise the behavior is undefined.
	 *
	 * \throws std::bad_alloc on allocation failure. Note: this pertains only to
	 *         CPU memory allocations. Failure to allocate GPU memory for the
	 *         texture data might not be reported directly.
	 *
	 * \sa grow2D()
	 */
	[[nodiscard]] Texture copy2D(Renderer& renderer) const;

	/**
	 * Get the floating-point size, in texels, of the 2D image data stored in
	 * this texture.
	 *
	 * \return a 2D vector representing the width and height of the texture, in
	 *         texels, or (0, 0) if the texture does not have a value.
	 *
	 * \note For 2D array textures, this function returns the width and height
	 *       of a single image layer in the array.
	 *
	 * \sa getWidth()
	 * \sa getHeight()
	 */
	[[nodiscard]] glm::vec2 getSize2D() const noexcept {
		return {static_cast<float>(width), static_cast<float>(height)};
	}

	/**
	 * Get the internal texel format of this texture.
	 *
	 * \return the internal texel format, or TextureInternalFormat::NONE if the
	 *         texture does not have a value.
	 */
	[[nodiscard]] TextureInternalFormat getInternalFormat() const noexcept {
		return internalFormat;
	}

	/**
	 * Get the width, in texels, of the 2D image data stored in this texture.
	 *
	 * \return the width of the texture, in texels, or 0 if the texture does not
	 *         have a value.
	 *
	 * \note For 2D array textures, this function returns the width of a single
	 *       image layer in the array.
	 *
	 * \sa getSize2D()
	 * \sa getHeight()
	 */
	[[nodiscard]] std::size_t getWidth() const noexcept {
		return width;
	}

	/**
	 * Get the height, in texels, of the 2D image data stored in this texture.
	 *
	 * \return the height of the texture, in texels, or 0 if the texture does
	 *         not have a value.
	 *
	 * \note For 2D array textures, this function returns the height of a single
	 *       image layer in the array.
	 *
	 * \sa getSize2D()
	 * \sa getWidth()
	 */
	[[nodiscard]] std::size_t getHeight() const noexcept {
		return height;
	}

	/**
	 * Get the configuration options of this texture and its associated sampler.
	 *
	 * \return a read-only reference to the internal texture options, valid
	 *         until the texture is moved from or destroyed.
	 */
	[[nodiscard]] const TextureOptions& getOptions() const noexcept {
		return options;
	}

	/**
	 * Get an opaque handle to the GPU representation of the texture.
	 *
	 * \return a non-owning resource handle to the GPU representation of the
	 *         texture.
	 *
	 * \note This function is used internally by the implementations of various
	 *       abstractions and is not intended to be used outside of the graphics
	 *       module. The returned handle has no meaning to application code.
	 */
	[[nodiscard]] Handle get() const noexcept {
		return texture.get();
	}

private:
	friend Renderer;

	static void createSharedTextures();
	static void destroySharedTextures() noexcept;

	struct TextureDeleter {
		void operator()(Handle handle) const noexcept;
	};

	Resource<Handle, TextureDeleter> texture{};
	TextureInternalFormat internalFormat = TextureInternalFormat::NONE;
	std::size_t width = 0;
	std::size_t height = 0;
	TextureOptions options{};
};

} // namespace graphics
} // namespace donut

#endif
