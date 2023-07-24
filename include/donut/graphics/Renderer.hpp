#ifndef DONUT_GRAPHICS_RENDERER_HPP
#define DONUT_GRAPHICS_RENDERER_HPP

#include <donut/graphics/Camera.hpp>
#include <donut/graphics/Framebuffer.hpp>
#include <donut/graphics/RenderPass.hpp>
#include <donut/graphics/Shader2D.hpp>
#include <donut/graphics/Shader3D.hpp>
#include <donut/graphics/Texture.hpp>
#include <donut/graphics/TexturedQuad.hpp>
#include <donut/graphics/Viewport.hpp>

namespace donut::graphics {

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
 * frame produced through the application::Application::display() callback. It
 * can also be used for smaller render jobs in the middle of a frame, such as
 * copying a GPU Texture.
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
	 * \param framebuffer framebuffer to clear the depth of.
	 *
	 * \sa clearFramebufferColor()
	 * \sa clearFramebufferColorAndDepth()
	 */
	void clearFramebufferDepth(Framebuffer& framebuffer);

	/**
	 * Clear the color contents of a Framebuffer.
	 *
	 * \param framebuffer framebuffer to clear the color of.
	 * \param color color to clear the framebuffer to.
	 *
	 * \sa clearFramebufferDepth()
	 * \sa clearFramebufferColorAndDepth()
	 */
	void clearFramebufferColor(Framebuffer& framebuffer, Color color);

	/**
	 * Clear the color and depth buffer contents of a Framebuffer.
	 *
	 * \param framebuffer framebuffer to clear the color and depth of.
	 * \param color color to clear the framebuffer to.
	 *
	 * \sa clearFramebufferDepth()
	 * \sa clearFramebufferColor()
	 */
	void clearFramebufferColorAndDepth(Framebuffer& framebuffer, Color color);

	/**
	 * Render the contents of a RenderPass to a Framebuffer.
	 *
	 * \param framebuffer framebuffer to render to.
	 * \param renderPass render pass to render from.
	 * \param viewport rectangular region of the framebuffer to render inside
	 *        of.
	 * \param camera camera to render from.
	 *
	 * \note This function should typically be called at least once every frame
	 *       during the application::Application::display() callback.
	 */
	void render(Framebuffer& framebuffer, const RenderPass& renderPass, const Viewport& viewport, const Camera& camera);

private:
	TexturedQuad texturedQuad{};
};

} // namespace donut::graphics

#endif
