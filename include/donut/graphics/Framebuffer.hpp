#ifndef DONUT_GRAPHICS_FRAMEBUFFER_HPP
#define DONUT_GRAPHICS_FRAMEBUFFER_HPP

#include <donut/Resource.hpp>
#include <donut/graphics/Handle.hpp>

namespace donut {
namespace graphics {

class Texture; // Forward declaration, to avoid including Texture.hpp.

/**
 * Handle that is either a non-owning handle to the default framebuffer or a
 * unique handle with exclusive ownership of a GPU framebuffer resource.
 */
class Framebuffer {
public:
	/**
	 * Scope guard type representing an active texture attachment to a
	 * framebuffer that automatically detaches itself when destroyed.
	 */
	class TextureAttachment {
	public:
		~TextureAttachment();

		TextureAttachment(const TextureAttachment&) = delete;
		TextureAttachment(TextureAttachment&&) = delete;
		TextureAttachment& operator=(const TextureAttachment&) = delete;
		TextureAttachment& operator=(TextureAttachment&&) = delete;

	private:
		friend Framebuffer;

		[[nodiscard]] TextureAttachment(Framebuffer& framebuffer, Texture& texture);

		Framebuffer& framebuffer;
	};

	/**
	 * Get a handle to the default framebuffer, which is usually the user's
	 * screen.
	 *
	 * \return a non-owning handle to the default framebuffer.
	 */
	[[nodiscard]] static Framebuffer getDefault() noexcept {
		return Framebuffer{Handle{}};
	}

	/**
	 * Create a new GPU framebuffer resource.
	 *
	 * \throws graphics::Error on failure to create the framebuffer object.
	 */
	Framebuffer();

	/**
	 * Attach a 2D texture to the color attachment of the framebuffer for
	 * drawing to.
	 *
	 * \param texture the texture to attach. Must be a valid 2D texture with a
	 *        framebuffer-compatible internal format, size and options.
	 *        Otherwise, the behavior is unspecified.
	 *
	 * \return a scope guard representing the texture attachment. The attachment
	 *         ends when the guard object is destroyed.
	 */
	[[nodiscard]] TextureAttachment attachTexture2D(Texture& texture) {
		return TextureAttachment{*this, texture};
	}

	/**
	 * Get an opaque handle to the GPU representation of the framebuffer.
	 *
	 * \return a non-owning resource handle to the GPU representation of the
	 *         framebuffer.
	 *
	 * \note This function is used internally by the implementations of various
	 *       abstractions and is not intended to be used outside of the graphics
	 *       module. The returned handle has no meaning to application code.
	 */
	[[nodiscard]] Handle get() const noexcept {
		return fbo.get();
	}

private:
	explicit Framebuffer(Handle handle) noexcept
		: fbo(handle) {}

	struct FramebufferDeleter {
		void operator()(Handle handle) const noexcept;
	};

	Resource<Handle, FramebufferDeleter> fbo{};
};

} // namespace graphics
} // namespace donut

#endif
