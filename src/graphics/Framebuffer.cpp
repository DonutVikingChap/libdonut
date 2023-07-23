#include <donut/graphics/Error.hpp>
#include <donut/graphics/Framebuffer.hpp>
#include <donut/graphics/Handle.hpp>
#include <donut/graphics/Texture.hpp>
#include <donut/graphics/opengl.hpp>

namespace donut::graphics {

Framebuffer::TextureAttachment::~TextureAttachment() {
	GLint oldFramebufferBinding = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFramebufferBinding);

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.get());
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(oldFramebufferBinding));
}

Framebuffer::TextureAttachment::TextureAttachment(Framebuffer& framebuffer, Texture& texture)
	: framebuffer(framebuffer) {
	GLint oldFramebufferBinding = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFramebufferBinding);

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.get());
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.get(), 0);

	glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(oldFramebufferBinding));
}

Framebuffer::Framebuffer() {
	Handle handle{};
	glGenFramebuffers(1, &handle);
	if (!handle) {
		throw Error{"Failed to create framebuffer object!"};
	}
	fbo.reset(handle);
}

void Framebuffer::FramebufferDeleter::operator()(Handle handle) const noexcept {
	glDeleteFramebuffers(1, &handle);
}

} // namespace donut::graphics
