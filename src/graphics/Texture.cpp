#include <donut/graphics/Camera.hpp>
#include <donut/graphics/Error.hpp>
#include <donut/graphics/Handle.hpp>
#include <donut/graphics/Image.hpp>
#include <donut/graphics/RenderPass.hpp>
#include <donut/graphics/Renderer.hpp>
#include <donut/graphics/Texture.hpp>
#include <donut/graphics/Viewport.hpp>
#include <donut/graphics/opengl.hpp>

#include <array>    // std::array
#include <cassert>  // assert
#include <cstddef>  // std::size_t, std::byte, std::max_align_t
#include <memory>   // std::construct_at, std::destroy_at
#include <optional> // std::optional

namespace donut::graphics {

namespace {

std::size_t sharedTextureReferenceCount = 0;
alignas(Texture) std::array<std::byte, sizeof(Texture)> sharedTransparentTextureStorage;
alignas(Texture) std::array<std::byte, sizeof(Texture)> sharedBlackTextureStorage;
alignas(Texture) std::array<std::byte, sizeof(Texture)> sharedWhiteTextureStorage;
alignas(Texture) std::array<std::byte, sizeof(Texture)> sharedDefaultSpecularTextureStorage;
alignas(Texture) std::array<std::byte, sizeof(Texture)> sharedDefaultNormalTextureStorage;

} // namespace

const Texture* const Texture::TRANSPARENT = reinterpret_cast<Texture*>(sharedTransparentTextureStorage.data());
const Texture* const Texture::BLACK = reinterpret_cast<Texture*>(sharedBlackTextureStorage.data());
const Texture* const Texture::WHITE = reinterpret_cast<Texture*>(sharedWhiteTextureStorage.data());
const Texture* const Texture::DEFAULT_SPECULAR = reinterpret_cast<Texture*>(sharedDefaultSpecularTextureStorage.data());
const Texture* const Texture::DEFAULT_NORMAL = reinterpret_cast<Texture*>(sharedDefaultNormalTextureStorage.data());

std::size_t Texture::getChannelCount(TextureFormat internalFormat) noexcept {
	switch (internalFormat) {
		case TextureFormat::R8_UNORM: [[fallthrough]];
		case TextureFormat::R16_FLOAT: [[fallthrough]];
		case TextureFormat::R32_FLOAT: return 1;
		case TextureFormat::R8G8_UNORM: [[fallthrough]];
		case TextureFormat::R16G16_FLOAT: [[fallthrough]];
		case TextureFormat::R32G32_FLOAT: return 2;
		case TextureFormat::R8G8B8_UNORM: [[fallthrough]];
		case TextureFormat::R16G16B16_FLOAT: [[fallthrough]];
		case TextureFormat::R32G32B32_FLOAT: return 3;
		case TextureFormat::R8G8B8A8_UNORM: [[fallthrough]];
		case TextureFormat::R16G16B16A16_FLOAT: [[fallthrough]];
		case TextureFormat::R32G32B32A32_FLOAT: return 4;
	}
	return 0;
}

PixelFormat Texture::getPixelFormat(TextureFormat internalFormat) noexcept {
	switch (internalFormat) {
		case TextureFormat::R8_UNORM: [[fallthrough]];
		case TextureFormat::R16_FLOAT: [[fallthrough]];
		case TextureFormat::R32_FLOAT: return PixelFormat::R;
		case TextureFormat::R8G8_UNORM: [[fallthrough]];
		case TextureFormat::R16G16_FLOAT: [[fallthrough]];
		case TextureFormat::R32G32_FLOAT: return PixelFormat::RG;
		case TextureFormat::R8G8B8_UNORM: [[fallthrough]];
		case TextureFormat::R16G16B16_FLOAT: [[fallthrough]];
		case TextureFormat::R32G32B32_FLOAT: return PixelFormat::RGB;
		case TextureFormat::R8G8B8A8_UNORM: [[fallthrough]];
		case TextureFormat::R16G16B16A16_FLOAT: [[fallthrough]];
		case TextureFormat::R32G32B32A32_FLOAT: return PixelFormat::RGBA;
	}
	return PixelFormat::R;
}

PixelComponentType Texture::getPixelComponentType(TextureFormat internalFormat) noexcept {
	switch (internalFormat) {
		case TextureFormat::R8_UNORM: [[fallthrough]];
		case TextureFormat::R8G8_UNORM: [[fallthrough]];
		case TextureFormat::R8G8B8_UNORM: [[fallthrough]];
		case TextureFormat::R8G8B8A8_UNORM: return PixelComponentType::U8;
		case TextureFormat::R16_FLOAT: [[fallthrough]];
		case TextureFormat::R16G16_FLOAT: [[fallthrough]];
		case TextureFormat::R16G16B16_FLOAT: [[fallthrough]];
		case TextureFormat::R16G16B16A16_FLOAT: return PixelComponentType::F16;
		case TextureFormat::R32_FLOAT: [[fallthrough]];
		case TextureFormat::R32G32_FLOAT: [[fallthrough]];
		case TextureFormat::R32G32B32_FLOAT: [[fallthrough]];
		case TextureFormat::R32G32B32A32_FLOAT: return PixelComponentType::F32;
	}
	return PixelComponentType::U8;
}

TextureFormat Texture::getInternalFormat(PixelFormat pixelFormat, PixelComponentType pixelComponentType) noexcept {
	switch (pixelFormat) {
		case PixelFormat::R:
			switch (pixelComponentType) {
				case PixelComponentType::U8: return TextureFormat::R8_UNORM;
				case PixelComponentType::F16: return TextureFormat::R16_FLOAT;
				case PixelComponentType::F32: return TextureFormat::R32_FLOAT;
			}
			break;
		case PixelFormat::RG:
			switch (pixelComponentType) {
				case PixelComponentType::U8: return TextureFormat::R8G8_UNORM;
				case PixelComponentType::F16: return TextureFormat::R16G16_FLOAT;
				case PixelComponentType::F32: return TextureFormat::R32G32_FLOAT;
			}
			break;
		case PixelFormat::RGB:
			switch (pixelComponentType) {
				case PixelComponentType::U8: return TextureFormat::R8G8B8_UNORM;
				case PixelComponentType::F16: return TextureFormat::R16G16B16_FLOAT;
				case PixelComponentType::F32: return TextureFormat::R32G32B32_FLOAT;
			}
			break;
		case PixelFormat::RGBA:
			switch (pixelComponentType) {
				case PixelComponentType::U8: return TextureFormat::R8G8B8A8_UNORM;
				case PixelComponentType::F16: return TextureFormat::R16G16B16A16_FLOAT;
				case PixelComponentType::F32: return TextureFormat::R32G32B32A32_FLOAT;
			}
			break;
	}
	return TextureFormat::R8_UNORM;
}

Texture::Texture(TextureFormat internalFormat, std::size_t width, std::size_t height, PixelFormat pixelFormat, PixelComponentType pixelComponentType, const void* pixels,
	const TextureOptions& options)
	: width(width)
	, height(height)
	, internalFormat(internalFormat) {
	Handle handle{};
	glGenTextures(1, &handle);
	if (!handle) {
		throw Error{"Failed to create texture object!"};
	}
	texture.reset(handle);

	GLint oldUnpackAlignment = 0;
	GLint oldTextureBinding2D = 0;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &oldUnpackAlignment);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTextureBinding2D);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, texture.get());
	glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(internalFormat), static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, static_cast<GLenum>(pixelFormat),
		static_cast<GLenum>(pixelComponentType), pixels);

	setOptions2D(options);

	glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(oldTextureBinding2D));
	glPixelStorei(GL_UNPACK_ALIGNMENT, oldUnpackAlignment);
}

Texture::Texture(TextureFormat internalFormat, std::size_t width, std::size_t height, std::size_t depth, PixelFormat pixelFormat, PixelComponentType pixelComponentType,
	const void* pixels, const TextureOptions& options)
	: width(width)
	, height(height)
	, internalFormat(internalFormat) {
	Handle handle{};
	glGenTextures(1, &handle);
	if (!handle) {
		throw Error{"Failed to create texture object!"};
	}
	texture.reset(handle);

	GLint oldUnpackAlignment = 0;
	GLint oldTextureBinding2DArray = 0;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &oldUnpackAlignment);
	glGetIntegerv(GL_TEXTURE_BINDING_2D_ARRAY, &oldTextureBinding2DArray);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture.get());
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, static_cast<GLint>(internalFormat), static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLsizei>(depth), 0,
		static_cast<GLenum>(pixelFormat), static_cast<GLenum>(pixelComponentType), pixels);

	setOptions2DArray(options);

	glBindTexture(GL_TEXTURE_2D_ARRAY, static_cast<GLuint>(oldTextureBinding2DArray));
	glPixelStorei(GL_UNPACK_ALIGNMENT, oldUnpackAlignment);
}

Texture::Texture(TextureFormat internalFormat, std::size_t width, std::size_t height, const TextureOptions& options)
	: Texture(internalFormat, width, height, getPixelFormat(internalFormat), getPixelComponentType(internalFormat), nullptr, options) {}

Texture::Texture(TextureFormat internalFormat, std::size_t width, std::size_t height, std::size_t depth, const TextureOptions& options)
	: Texture(internalFormat, width, height, depth, getPixelFormat(internalFormat), getPixelComponentType(internalFormat), nullptr, options) {}

Texture::Texture(const ImageView& image, const TextureOptions& options)
	: Texture(getInternalFormat(image.getPixelFormat(), image.getPixelComponentType()), image.getWidth(), image.getHeight(), image.getPixelFormat(), image.getPixelComponentType(),
		  image.getPixels(), options) {}

void Texture::setOptions2D(const TextureOptions& newOptions) {
	options = newOptions;

	GLint oldTextureBinding2D = 0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTextureBinding2D);

	glBindTexture(GL_TEXTURE_2D, texture.get());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (options.repeat) ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (options.repeat) ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	if (options.useMipmap) {
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (options.useLinearFiltering) ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (options.useLinearFiltering) ? GL_LINEAR : GL_NEAREST);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (options.useLinearFiltering) ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (options.useLinearFiltering) ? GL_LINEAR : GL_NEAREST);
	}

	glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(oldTextureBinding2D));
}

void Texture::setOptions2DArray(const TextureOptions& newOptions) {
	options = newOptions;

	GLint oldTextureBinding2DArray = 0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D_ARRAY, &oldTextureBinding2DArray);

	glBindTexture(GL_TEXTURE_2D_ARRAY, texture.get());

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, (options.repeat) ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, (options.repeat) ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, (options.useLinearFiltering) ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, (options.useLinearFiltering) ? GL_LINEAR : GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D_ARRAY, static_cast<GLuint>(oldTextureBinding2DArray));
}

void Texture::pasteImage2D(std::size_t imageWidth, std::size_t imageHeight, PixelFormat pixelFormat, PixelComponentType pixelComponentType, const void* pixels, std::size_t x,
	std::size_t y) {
	GLint oldUnpackAlignment = 0;
	GLint oldTextureBinding2D = 0;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &oldUnpackAlignment);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTextureBinding2D);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, texture.get());
	glTexSubImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(imageWidth), static_cast<GLsizei>(imageHeight),
		static_cast<GLenum>(pixelFormat), static_cast<GLenum>(pixelComponentType), pixels);

	glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(oldTextureBinding2D));
	glPixelStorei(GL_UNPACK_ALIGNMENT, oldUnpackAlignment);
}

void Texture::pasteImage2D(const ImageView& image, std::size_t x, std::size_t y) {
	pasteImage2D(image.getWidth(), image.getHeight(), image.getPixelFormat(), image.getPixelComponentType(), image.getPixels(), x, y);
}

void Texture::pasteImage2DArray(std::size_t imageWidth, std::size_t imageHeight, std::size_t arrayDepth, PixelFormat pixelFormat, PixelComponentType pixelComponentType,
	const void* pixels, std::size_t x, std::size_t y, std::size_t z) {
	GLint oldUnpackAlignment = 0;
	GLint oldTextureBinding2DArray = 0;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &oldUnpackAlignment);
	glGetIntegerv(GL_TEXTURE_BINDING_2D_ARRAY, &oldTextureBinding2DArray);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture.get());
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLint>(z), static_cast<GLsizei>(imageWidth),
		static_cast<GLsizei>(imageHeight), static_cast<GLsizei>(arrayDepth), static_cast<GLenum>(pixelFormat), static_cast<GLenum>(pixelComponentType), pixels);

	glBindTexture(GL_TEXTURE_2D_ARRAY, static_cast<GLuint>(oldTextureBinding2DArray));
	glPixelStorei(GL_UNPACK_ALIGNMENT, oldUnpackAlignment);
}

void Texture::pasteImage2DArray(const ImageView& image, std::size_t x, std::size_t y, std::size_t z) {
	pasteImage2DArray(image.getWidth(), image.getHeight(), 1, image.getPixelFormat(), image.getPixelComponentType(), image.getPixels(), x, y, z);
}

void Texture::fill2D(Renderer& renderer, Color color) {
	Framebuffer framebuffer{};
	const Framebuffer::TextureAttachment attachment = framebuffer.attachTexture2D(*this);
	renderer.clearFramebufferColor(framebuffer, color);
}

void Texture::grow2D(Renderer& renderer, std::size_t newWidth, std::size_t newHeight, std::optional<Color> backgroundColor) {
	assert(newWidth >= width);
	assert(newHeight >= height);
	*this = copyGrow2D(renderer, newWidth, newHeight, backgroundColor);
}

Texture Texture::copy2D(Renderer& renderer) const {
	return copyGrow2D(renderer, width, height);
}

Texture Texture::copyGrow2D(Renderer& renderer, std::size_t newWidth, std::size_t newHeight, std::optional<Color> backgroundColor) const {
	assert(newWidth >= width);
	assert(newHeight >= height);
	Texture newTexture{internalFormat, newWidth, newHeight, {.repeat = false, .useLinearFiltering = false, .useMipmap = false}};
	{
		Framebuffer framebuffer{};
		const Framebuffer::TextureAttachment attachment = framebuffer.attachTexture2D(newTexture);
		if (backgroundColor) {
			renderer.clearFramebufferColor(framebuffer, *backgroundColor);
		}
		alignas(std::max_align_t) std::array<std::byte, 128> renderPassStorage;
		renderer.render(framebuffer, RenderPass{renderPassStorage}.draw(TextureInstance{.texture = this}),
			Viewport{.position{0, 0}, .size{static_cast<GLint>(width), static_cast<GLint>(height)}},
			Camera::createOrthographic({.offset{0.0f, 0.0f}, .size{static_cast<float>(width), static_cast<float>(height)}}));
	}
	newTexture.setOptions2D(options);
	return newTexture;
}

void Texture::createSharedTextures() {
	if (sharedTextureReferenceCount == 0) {
		constexpr std::array<std::byte, 4> TRANSPARENT_PIXEL{std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0}};
		constexpr std::array<std::byte, 4> BLACK_PIXEL{std::byte{0}, std::byte{0}, std::byte{0}, std::byte{255}};
		constexpr std::array<std::byte, 4> WHITE_PIXEL{std::byte{255}, std::byte{255}, std::byte{255}, std::byte{255}};
		constexpr std::array<std::byte, 4> DEFAULT_SPECULAR_PIXEL{std::byte{16}, std::byte{16}, std::byte{16}, std::byte{255}};
		constexpr std::array<std::byte, 4> DEFAULT_NORMAL_PIXEL{std::byte{128}, std::byte{128}, std::byte{255}, std::byte{255}};
		constexpr TextureOptions PIXEL_TEXTURE_OPTIONS{.repeat = true, .useLinearFiltering = false, .useMipmap = false};
		std::construct_at(const_cast<Texture*>(TRANSPARENT), TextureFormat::R8G8B8A8_UNORM, 1, 1, PixelFormat::RGBA, PixelComponentType::U8, TRANSPARENT_PIXEL.data(),
			PIXEL_TEXTURE_OPTIONS);
		try {
			std::construct_at(const_cast<Texture*>(BLACK), TextureFormat::R8G8B8A8_UNORM, 1, 1, PixelFormat::RGBA, PixelComponentType::U8, BLACK_PIXEL.data(),
				PIXEL_TEXTURE_OPTIONS);
			try {
				std::construct_at(const_cast<Texture*>(WHITE), TextureFormat::R8G8B8A8_UNORM, 1, 1, PixelFormat::RGBA, PixelComponentType::U8, WHITE_PIXEL.data(),
					PIXEL_TEXTURE_OPTIONS);
				try {
					std::construct_at(const_cast<Texture*>(DEFAULT_SPECULAR), TextureFormat::R8G8B8A8_UNORM, 1, 1, PixelFormat::RGBA, PixelComponentType::U8,
						DEFAULT_SPECULAR_PIXEL.data(), PIXEL_TEXTURE_OPTIONS);
					try {
						std::construct_at(const_cast<Texture*>(DEFAULT_NORMAL), TextureFormat::R8G8B8A8_UNORM, 1, 1, PixelFormat::RGBA, PixelComponentType::U8,
							DEFAULT_NORMAL_PIXEL.data(), PIXEL_TEXTURE_OPTIONS);
					} catch (...) {
						std::destroy_at(DEFAULT_SPECULAR);
						throw;
					}
				} catch (...) {
					std::destroy_at(WHITE);
					throw;
				}
			} catch (...) {
				std::destroy_at(BLACK);
				throw;
			}
		} catch (...) {
			std::destroy_at(TRANSPARENT);
			throw;
		}
	}
	++sharedTextureReferenceCount;
}

void Texture::destroySharedTextures() noexcept {
	if (sharedTextureReferenceCount-- == 1) {
		std::destroy_at(DEFAULT_NORMAL);
		std::destroy_at(DEFAULT_SPECULAR);
		std::destroy_at(WHITE);
		std::destroy_at(BLACK);
		std::destroy_at(TRANSPARENT);
	}
}

void Texture::TextureDeleter::operator()(Handle handle) const noexcept {
	glDeleteTextures(1, &handle);
}

} // namespace donut::graphics
