#include <donut/graphics/Error.hpp>
#include <donut/graphics/Handle.hpp>
#include <donut/graphics/ImageHDR.hpp>
#include <donut/graphics/ImageLDR.hpp>
#include <donut/graphics/RenderPass.hpp>
#include <donut/graphics/Renderer.hpp>
#include <donut/graphics/Texture.hpp>
#include <donut/graphics/opengl.hpp>

#include <cassert>                       // assert
#include <cstddef>                       // std::size_t
#include <fmt/format.h>                  // fmt::format
#include <glm/ext/matrix_clip_space.hpp> // glm::ortho
#include <optional>                      // std::optional
#include <utility>                       // std::move

namespace donut {
namespace graphics {

std::size_t Texture::getChannelCount(TextureFormat format) {
	switch (format) {
		case TextureFormat::NONE: return 0;
		case TextureFormat::R: return 1;
		case TextureFormat::RG: return 2;
		case TextureFormat::RGB: return 3;
		case TextureFormat::RGBA: return 4;
	}
	return 0;
}

std::size_t Texture::getInternalChannelCount(TextureInternalFormat internalFormat) {
	switch (internalFormat) {
		case TextureInternalFormat::NONE: return 0;
		case TextureInternalFormat::R8: [[fallthrough]];
		case TextureInternalFormat::R16F: [[fallthrough]];
		case TextureInternalFormat::R32F: return 1;
		case TextureInternalFormat::RG8: [[fallthrough]];
		case TextureInternalFormat::RG16F: [[fallthrough]];
		case TextureInternalFormat::RG32F: return 2;
		case TextureInternalFormat::RGB8: [[fallthrough]];
		case TextureInternalFormat::RGB16F: [[fallthrough]];
		case TextureInternalFormat::RGB32F: return 3;
		case TextureInternalFormat::RGBA8: [[fallthrough]];
		case TextureInternalFormat::RGBA16F: [[fallthrough]];
		case TextureInternalFormat::RGBA32F: return 4;
	}
	return 0;
}

TextureComponentType Texture::getInternalComponentType(TextureInternalFormat internalFormat) {
	switch (internalFormat) {
		case TextureInternalFormat::NONE: return TextureComponentType::U8;
		case TextureInternalFormat::R8: [[fallthrough]];
		case TextureInternalFormat::RG8: [[fallthrough]];
		case TextureInternalFormat::RGB8: [[fallthrough]];
		case TextureInternalFormat::RGBA8: return TextureComponentType::U8;
		case TextureInternalFormat::R16F: [[fallthrough]];
		case TextureInternalFormat::RG16F: [[fallthrough]];
		case TextureInternalFormat::RGB16F: [[fallthrough]];
		case TextureInternalFormat::RGBA16F: return TextureComponentType::F16;
		case TextureInternalFormat::R32F: [[fallthrough]];
		case TextureInternalFormat::RG32F: [[fallthrough]];
		case TextureInternalFormat::RGB32F: [[fallthrough]];
		case TextureInternalFormat::RGBA32F: return TextureComponentType::F32;
	}
	return TextureComponentType::U8;
}

TextureFormat Texture::getPixelFormat(std::size_t channelCount) {
	switch (channelCount) {
		case 1: return TextureFormat::R;
		case 2: return TextureFormat::RG;
		case 3: return TextureFormat::RGB;
		case 4: return TextureFormat::RGBA;
		default: break;
	}
	throw Error{fmt::format("Invalid texture channel count \"{}\"!", channelCount)};
}

TextureInternalFormat Texture::getInternalPixelFormatU8(std::size_t channelCount) {
	switch (channelCount) {
		case 1: return TextureInternalFormat::R8;
		case 2: return TextureInternalFormat::RG8;
		case 3: return TextureInternalFormat::RGB8;
		case 4: return TextureInternalFormat::RGBA8;
		default: break;
	}
	throw Error{fmt::format("Invalid texture channel count \"{}\"!", channelCount)};
}

TextureInternalFormat Texture::getInternalPixelFormatF16(std::size_t channelCount) {
	switch (channelCount) {
		case 1: return TextureInternalFormat::R16F;
		case 2: return TextureInternalFormat::RG16F;
		case 3: return TextureInternalFormat::RGB16F;
		case 4: return TextureInternalFormat::RGBA16F;
		default: break;
	}
	throw Error{fmt::format("Invalid texture channel count \"{}\"!", channelCount)};
}

TextureInternalFormat Texture::getInternalPixelFormatF32(std::size_t channelCount) {
	switch (channelCount) {
		case 1: return TextureInternalFormat::R32F;
		case 2: return TextureInternalFormat::RG32F;
		case 3: return TextureInternalFormat::RGB32F;
		case 4: return TextureInternalFormat::RGBA32F;
		default: break;
	}
	throw Error{fmt::format("Invalid texture channel count \"{}\"!", channelCount)};
}

Texture::Texture(
	TextureInternalFormat internalFormat, std::size_t width, std::size_t height, TextureFormat format, TextureComponentType type, const void* pixels, const TextureOptions& options)
	: internalFormat(internalFormat)
	, width(width)
	, height(height) {
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
	glTexImage2D(GL_TEXTURE_2D,
		0,
		static_cast<GLint>(internalFormat),
		static_cast<GLsizei>(width),
		static_cast<GLsizei>(height),
		0,
		static_cast<GLenum>(format),
		static_cast<GLenum>(type),
		pixels);

	setOptions2D(options);

	glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(oldTextureBinding2D));
	glPixelStorei(GL_UNPACK_ALIGNMENT, oldUnpackAlignment);
}

Texture::Texture(TextureInternalFormat internalFormat, std::size_t width, std::size_t height, std::size_t depth, TextureFormat format, TextureComponentType type,
	const void* pixels, const TextureOptions& options)
	: internalFormat(internalFormat)
	, width(width)
	, height(height) {
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
	glTexImage3D(GL_TEXTURE_2D_ARRAY,
		0,
		static_cast<GLint>(internalFormat),
		static_cast<GLsizei>(width),
		static_cast<GLsizei>(height),
		static_cast<GLsizei>(depth),
		0,
		static_cast<GLenum>(format),
		static_cast<GLenum>(type),
		pixels);

	setOptions2DArray(options);

	glBindTexture(GL_TEXTURE_2D_ARRAY, static_cast<GLuint>(oldTextureBinding2DArray));
	glPixelStorei(GL_UNPACK_ALIGNMENT, oldUnpackAlignment);
}

Texture::Texture(TextureInternalFormat internalFormat, std::size_t width, std::size_t height, const TextureOptions& options)
	: Texture(internalFormat, width, height, getPixelFormat(getInternalChannelCount(internalFormat)), getInternalComponentType(internalFormat), nullptr, options) {}

Texture::Texture(const ImageLDRView& image, const TextureOptions& options)
	: Texture(getInternalPixelFormatU8(image.getChannelCount()), image.getWidth(), image.getHeight(), getPixelFormat(image.getChannelCount()), TextureComponentType::U8,
		  image.getPixels(), options) {}

Texture::Texture(const ImageHDRView& image, const TextureOptions& options)
	: Texture(getInternalPixelFormatF16(image.getChannelCount()), image.getWidth(), image.getHeight(), getPixelFormat(image.getChannelCount()), TextureComponentType::F32,
		  image.getPixels(),
		  {
			  .repeat = options.repeat,
			  .useLinearFiltering = options.useLinearFiltering,
			  .useMipmap = false,
		  }) {}

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

void Texture::pasteImage2D(std::size_t width, std::size_t height, TextureFormat format, TextureComponentType type, const void* pixels, std::size_t x, std::size_t y) {
	GLint oldUnpackAlignment = 0;
	GLint oldTextureBinding2D = 0;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &oldUnpackAlignment);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTextureBinding2D);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, texture.get());
	glTexSubImage2D(GL_TEXTURE_2D,
		0,
		static_cast<GLint>(x),
		static_cast<GLint>(y),
		static_cast<GLsizei>(width),
		static_cast<GLsizei>(height),
		static_cast<GLenum>(format),
		static_cast<GLenum>(type),
		pixels);

	glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(oldTextureBinding2D));
	glPixelStorei(GL_UNPACK_ALIGNMENT, oldUnpackAlignment);
}

void Texture::pasteImage2D(const ImageLDRView& image, std::size_t x, std::size_t y) {
	pasteImage2D(image.getWidth(), image.getHeight(), getPixelFormat(image.getChannelCount()), TextureComponentType::U8, image.getPixels(), x, y);
}

void Texture::pasteImage2D(const ImageHDRView& image, std::size_t x, std::size_t y) {
	pasteImage2D(image.getWidth(), image.getHeight(), getPixelFormat(image.getChannelCount()), TextureComponentType::F32, image.getPixels(), x, y);
}

void Texture::pasteImage2DArray(
	std::size_t width, std::size_t height, std::size_t depth, TextureFormat format, TextureComponentType type, const void* pixels, std::size_t x, std::size_t y, std::size_t z) {
	GLint oldUnpackAlignment = 0;
	GLint oldTextureBinding2DArray = 0;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &oldUnpackAlignment);
	glGetIntegerv(GL_TEXTURE_BINDING_2D_ARRAY, &oldTextureBinding2DArray);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture.get());
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
		0,
		static_cast<GLint>(x),
		static_cast<GLint>(y),
		static_cast<GLint>(z),
		static_cast<GLsizei>(width),
		static_cast<GLsizei>(height),
		static_cast<GLsizei>(depth),
		static_cast<GLenum>(format),
		static_cast<GLenum>(type),
		pixels);

	glBindTexture(GL_TEXTURE_2D_ARRAY, static_cast<GLuint>(oldTextureBinding2DArray));
	glPixelStorei(GL_UNPACK_ALIGNMENT, oldUnpackAlignment);
}

void Texture::pasteImage2DArray(const ImageLDRView& image, std::size_t x, std::size_t y, std::size_t z) {
	pasteImage2DArray(image.getWidth(), image.getHeight(), 1, getPixelFormat(image.getChannelCount()), TextureComponentType::U8, image.getPixels(), x, y, z);
}

void Texture::pasteImage2DArray(const ImageHDRView& image, std::size_t x, std::size_t y, std::size_t z) {
	pasteImage2DArray(image.getWidth(), image.getHeight(), 1, getPixelFormat(image.getChannelCount()), TextureComponentType::F32, image.getPixels(), x, y, z);
}

void Texture::fill2D(Renderer& renderer, Color color) {
	Framebuffer framebuffer{};
	const Framebuffer::TextureAttachment attachment = framebuffer.attachTexture2D(*this);
	renderer.clearFramebufferColor(framebuffer, color);
}

void Texture::grow2D(Renderer& renderer, std::size_t newWidth, std::size_t newHeight, std::optional<Color> backgroundColor) {
	assert(newWidth >= width);
	assert(newHeight >= height);

	Texture newTexture{internalFormat, newWidth, newHeight, {.repeat = false, .useLinearFiltering = false, .useMipmap = false}};
	{
		Framebuffer framebuffer{};
		const Framebuffer::TextureAttachment attachment = framebuffer.attachTexture2D(newTexture);

		if (backgroundColor) {
			renderer.clearFramebufferColor(framebuffer, *backgroundColor);
		}

		RenderPass renderPass{};
		renderPass.draw(TextureInstance{.texture = this});
		renderer.render(framebuffer,
			renderPass,
			{.position{0, 0}, .size{static_cast<GLint>(width), static_cast<GLint>(height)}},
			glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height)));
	}
	newTexture.setOptions2D(options);
	*this = std::move(newTexture);
}

Texture Texture::copy2D(Renderer& renderer) const {
	Texture newTexture{internalFormat, width, height, {.repeat = false, .useLinearFiltering = false, .useMipmap = false}};
	{
		Framebuffer framebuffer{};
		const Framebuffer::TextureAttachment attachment = framebuffer.attachTexture2D(newTexture);

		RenderPass renderPass{};
		renderPass.draw(TextureInstance{.texture = this});
		renderer.render(framebuffer,
			renderPass,
			{.position{0, 0}, .size{static_cast<GLint>(width), static_cast<GLint>(height)}},
			glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height)));
	}
	newTexture.setOptions2D(options);
	return newTexture;
}

void Texture::TextureDeleter::operator()(Handle handle) const noexcept {
	glDeleteTextures(1, &handle);
}

} // namespace graphics
} // namespace donut
