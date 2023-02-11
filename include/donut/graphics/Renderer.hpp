#ifndef DONUT_GRAPHICS_RENDERER_HPP
#define DONUT_GRAPHICS_RENDERER_HPP

#include <donut/graphics/Framebuffer.hpp>
#include <donut/graphics/RenderPass.hpp>
#include <donut/graphics/Shader2D.hpp>
#include <donut/graphics/Shader3D.hpp>
#include <donut/graphics/Texture.hpp>
#include <donut/graphics/TexturedQuad.hpp>
#include <donut/graphics/Viewport.hpp>

#include <array>       // std::array
#include <cstddef>     // std::byte
#include <glm/glm.hpp> // glm::...

namespace donut {
namespace graphics {

/**
 * Configuration options for a Renderer.
 */
struct RendererOptions {};

/**
 * Persistent system for rendering the batched draw commands of a RenderPass
 * onto a Framebuffer, such as the user's screen.
 *
 * An instance of this class should be kept throughout the lifetime of the
 * application in order to continuously render the visual state of the latest
 * frame produced through the Application::display() callback. It can also be
 * used for smaller render jobs in the middle of a frame, such as copying a
 * GPU Texture.
 *
 * \sa RenderPass
 */
class Renderer {
public:
	/**
	 * Construct a renderer.
	 *
	 * \param options initial configuration of the renderer, see
	 *        RendererOptions.
	 *
	 * \throws File::Error on failure to open a required resource file.
	 * \throws graphics::Error on failure to create or initialize a required GPU
	 *         resource.
	 * \throws std::bad_alloc on allocation failure.
	 */
	explicit Renderer(const RendererOptions& options = {});

	/** Destructor. */
	~Renderer();

	/** Copying a renderer is not allowed, since it manages global state. */
	Renderer(const Renderer&) = delete;

	/** Moving a renderer is not allowed, since it manages global state. */
	Renderer(Renderer&&) = delete;

	/** Copying a renderer is not allowed, since it manages global state. */
	Renderer& operator=(const Renderer&) = delete;

	/** Moving a renderer is not allowed, since it manages global state. */
	Renderer operator=(Renderer&&) = delete;

	/**
	 * Clear the depth buffer contents of a Framebuffer.
	 *
	 * \param framebuffer the framebuffer to clear the depth of.
	 *
	 * \sa clearFramebufferColor()
	 * \sa clearFramebufferColorAndDepth()
	 */
	void clearFramebufferDepth(Framebuffer& framebuffer);

	/**
	 * Clear the color contents of a Framebuffer.
	 *
	 * \param framebuffer the framebuffer to clear the color of.
	 * \param color color to clear the framebuffer to.
	 *
	 * \sa clearFramebufferDepth()
	 * \sa clearFramebufferColorAndDepth()
	 */
	void clearFramebufferColor(Framebuffer& framebuffer, Color color);

	/**
	 * Clear the color and depth buffer contents of a Framebuffer.
	 *
	 * \param framebuffer the framebuffer to clear the color and depth of.
	 * \param color color to clear the framebuffer to.
	 *
	 * \sa clearFramebufferDepth()
	 * \sa clearFramebufferColor()
	 */
	void clearFramebufferColorAndDepth(Framebuffer& framebuffer, Color color);

	/**
	 * Render the contents of a RenderPass to a Framebuffer.
	 *
	 * \param framebuffer the framebuffer to render to.
	 * \param renderPass the render pass to render from.
	 * \param viewport the rectangular region of the framebuffer to render
	 *        inside of.
	 * \param projectionViewMatrix combined projection/view matrix to transform
	 *        all rendered vertices by.
	 *
	 * \note This function should typically be called at least once every frame
	 *       during the Application::display() callback.
	 */
	void render(Framebuffer& framebuffer, const RenderPass& renderPass, const Viewport& viewport, const glm::mat4& projectionViewMatrix);

private:
	TexturedQuad texturedQuad{};
};

} // namespace graphics
} // namespace donut

#endif
