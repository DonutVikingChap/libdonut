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

class Renderer;
class ImageLDRView;
class ImageHDRView;

enum class TextureFormat : std::uint32_t {
	NONE = 0,
	R = 0x1903,
	RG = 0x8227,
	RGB = 0x1907,
	RGBA = 0x1908,
};

enum class TextureInternalFormat : std::int32_t {
	NONE = 0,
	R8 = 0x8229,
	RG8 = 0x822B,
	RGB8 = 0x8051,
	RGBA8 = 0x8058,
	R16F = 0x822D,
	RG16F = 0x822F,
	RGB16F = 0x881B,
	RGBA16F = 0x881A,
	R32F = 0x822E,
	RG32F = 0x8230,
	RGB32F = 0x8815,
	RGBA32F = 0x8814,
};

enum class TextureComponentType : std::uint32_t {
	U8 = 0x1401,
	F16 = 0x140B,
	F32 = 0x1406,
};

struct TextureOptions {
	bool repeat = true;
	bool useLinearFiltering = true;
	bool useMipmap = true;
};

class Texture {
public:
	[[nodiscard]] static std::size_t getChannelCount(TextureFormat format);
	[[nodiscard]] static std::size_t getInternalChannelCount(TextureInternalFormat internalFormat);
	[[nodiscard]] static TextureComponentType getInternalComponentType(TextureInternalFormat internalFormat);

	[[nodiscard]] static TextureFormat getPixelFormat(std::size_t channelCount);
	[[nodiscard]] static TextureInternalFormat getInternalPixelFormatU8(std::size_t channelCount);
	[[nodiscard]] static TextureInternalFormat getInternalPixelFormatF16(std::size_t channelCount);
	[[nodiscard]] static TextureInternalFormat getInternalPixelFormatF32(std::size_t channelCount);

	Texture() noexcept = default;

	Texture(TextureInternalFormat internalFormat, std::size_t width, std::size_t height, TextureFormat format, TextureComponentType type, const void* pixels,
		const TextureOptions& options = {});

	Texture(TextureInternalFormat internalFormat, std::size_t width, std::size_t height, std::size_t depth, TextureFormat format, TextureComponentType type, const void* pixels,
		const TextureOptions& options = {});

	Texture(TextureInternalFormat internalFormat, std::size_t width, std::size_t height, const TextureOptions& options = {});

	Texture(const ImageLDRView& image, const TextureOptions& options = {});
	Texture(const ImageHDRView& image, const TextureOptions& options = {});

	explicit operator bool() const noexcept {
		return static_cast<bool>(texture);
	}

	void reset() noexcept {
		*this = Texture{};
	}

	void setOptions2D(const TextureOptions& newOptions);
	void setOptions2DArray(const TextureOptions& newOptions);

	void pasteImage2D(std::size_t width, std::size_t height, TextureFormat format, TextureComponentType type, const void* pixels, std::size_t x, std::size_t y);
	void pasteImage2D(const ImageLDRView& image, std::size_t x, std::size_t y);
	void pasteImage2D(const ImageHDRView& image, std::size_t x, std::size_t y);

	void pasteImage2DArray(
		std::size_t width, std::size_t height, std::size_t depth, TextureFormat format, TextureComponentType type, const void* pixels, std::size_t x, std::size_t y, std::size_t z);
	void pasteImage2DArray(const ImageLDRView& image, std::size_t x, std::size_t y, std::size_t z);
	void pasteImage2DArray(const ImageHDRView& image, std::size_t x, std::size_t y, std::size_t z);

	void fill2D(Renderer& renderer, Color color);

	void grow2D(Renderer& renderer, std::size_t newWidth, std::size_t newHeight, std::optional<Color> backgroundColor = {});

	[[nodiscard]] Texture copy2D(Renderer& renderer) const;

	[[nodiscard]] glm::vec2 getSize() const noexcept {
		return {static_cast<float>(width), static_cast<float>(height)};
	}

	[[nodiscard]] TextureInternalFormat getInternalFormat() const noexcept {
		return internalFormat;
	}

	[[nodiscard]] std::size_t getWidth() const noexcept {
		return width;
	}

	[[nodiscard]] std::size_t getHeight() const noexcept {
		return height;
	}

	[[nodiscard]] const TextureOptions& getOptions() const noexcept {
		return options;
	}

	[[nodiscard]] Handle get() const noexcept {
		return texture.get();
	}

private:
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
