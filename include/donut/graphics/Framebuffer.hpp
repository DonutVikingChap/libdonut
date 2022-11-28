#ifndef DONUT_GRAPHICS_FRAMEBUFFER_HPP
#define DONUT_GRAPHICS_FRAMEBUFFER_HPP

#include <donut/Resource.hpp>
#include <donut/graphics/Handle.hpp>

namespace donut {
namespace graphics {

class Texture;

class Framebuffer {
public:
	[[nodiscard]] static Framebuffer& getDefault();

	Framebuffer();

	void attachTexture2D(Texture& texture);
	void detachTexture2D();

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
