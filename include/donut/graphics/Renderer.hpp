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
struct RendererOptions {
	/**
	 * Configuration options for the ShaderProgram of the default shader, which
	 * is used for rendering plain textured quads.
	 */
	ShaderProgramOptions defaultShaderProgramOptions{
		.vertexShaderSourceCode = Shader2D::vertexShaderSourceCodeInstancedTexturedQuad,
		.fragmentShaderSourceCode = Shader2D::fragmentShaderSourceCodeTexturedQuadPlain,
	};

	/**
	 * Configuration options for the Shader2D of the default shader, which is
	 * used for rendering plain textured quads.
	 */
	Shader2DOptions defaultShaderOptions{.orderIndex = 0};

	/**
	 * Configuration options for the ShaderProgram of the glyph shader, which is
	 * used for rendering the individual glyphs of shaped text.
	 */
	ShaderProgramOptions glyphShaderProgramOptions{
		.vertexShaderSourceCode = Shader2D::vertexShaderSourceCodeInstancedTexturedQuad,
		.fragmentShaderSourceCode = Shader2D::fragmentShaderSourceCodeTexturedQuadAlpha,
	};

	/**
	 * Configuration options for the Shader2D of the glyph shader, which is
	 * used for rendering the individual glyphs of shaped text.
	 */
	Shader2DOptions glyphShaderOptions{.orderIndex = 0};

	/**
	 * Configuration options for the ShaderProgram of the model shader, which is
	 * used for rendering 3D models.
	 */
	ShaderProgramOptions modelShaderProgramOptions{
		.vertexShaderSourceCode = Shader3D::vertexShaderSourceCodeInstancedModel,
		.fragmentShaderSourceCode = Shader3D::fragmentShaderSourceCodeModelBlinnPhong,
	};

	/**
	 * Configuration options for the Shader3D of the model shader, which is
	 * used for rendering 3D models.
	 */
	Shader3DOptions modelShaderOptions{.orderIndex = 0};
};

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
	explicit Renderer(const RendererOptions& options = {})
		: defaultShader(options.defaultShaderProgramOptions, options.defaultShaderOptions)
		, glyphShader(options.glyphShaderProgramOptions, options.glyphShaderOptions)
		, modelShader(options.modelShaderProgramOptions, options.modelShaderOptions) {}

	/**
	 * Clear the depth buffer contents of a Framebuffer.
	 *
	 * \param framebuffer the framebuffer to clear the depth of.
	 *
	 * \sa clearFramebufferStencil()
	 * \sa clearFramebufferDepthAndStencil()
	 * \sa clearFramebufferColor()
	 * \sa clearFramebufferColorAndDepth()
	 * \sa clearFramebufferColorAndStencil()
	 * \sa clearFramebufferColorAndDepthAndStencil()
	 */
	void clearFramebufferDepth(Framebuffer& framebuffer);

	/**
	 * Clear the stencil buffer contents of a Framebuffer.
	 *
	 * \param framebuffer the framebuffer to clear the stencil of.
	 * \param stencilValue value to fill the stencil buffer with.
	 *
	 * \sa clearFramebufferDepth()
	 * \sa clearFramebufferDepthAndStencil()
	 * \sa clearFramebufferColor()
	 * \sa clearFramebufferColorAndDepth()
	 * \sa clearFramebufferColorAndStencil()
	 * \sa clearFramebufferColorAndDepthAndStencil()
	 */
	void clearFramebufferStencil(Framebuffer& framebuffer, std::int32_t stencilValue);

	/**
	 * Clear the depth and stencil buffer contents of a Framebuffer.
	 *
	 * \param framebuffer the framebuffer to clear the depth and stencil of.
	 * \param stencilValue value to fill the stencil buffer with.
	 *
	 * \sa clearFramebufferDepth()
	 * \sa clearFramebufferStencil()
	 * \sa clearFramebufferColor()
	 * \sa clearFramebufferColorAndDepth()
	 * \sa clearFramebufferColorAndStencil()
	 * \sa clearFramebufferColorAndDepthAndStencil()
	 */
	void clearFramebufferDepthAndStencil(Framebuffer& framebuffer, std::int32_t stencilValue);

	/**
	 * Clear the color contents of a Framebuffer.
	 *
	 * \param framebuffer the framebuffer to clear the color of.
	 * \param color color to clear the framebuffer to.
	 *
	 * \sa clearFramebufferDepth()
	 * \sa clearFramebufferStencil()
	 * \sa clearFramebufferDepthAndStencil()
	 * \sa clearFramebufferColorAndDepth()
	 * \sa clearFramebufferColorAndStencil()
	 * \sa clearFramebufferColorAndDepthAndStencil()
	 */
	void clearFramebufferColor(Framebuffer& framebuffer, Color color);

	/**
	 * Clear the color and depth buffer contents of a Framebuffer.
	 *
	 * \param framebuffer the framebuffer to clear the color and depth of.
	 * \param color color to clear the framebuffer to.
	 *
	 * \sa clearFramebufferDepth()
	 * \sa clearFramebufferStencil()
	 * \sa clearFramebufferDepthAndStencil()
	 * \sa clearFramebufferColor()
	 * \sa clearFramebufferColorAndStencil()
	 * \sa clearFramebufferColorAndDepthAndStencil()
	 */
	void clearFramebufferColorAndDepth(Framebuffer& framebuffer, Color color);

	/**
	 * Clear the color and stencil buffer contents of a Framebuffer.
	 *
	 * \param framebuffer the framebuffer to clear the color and stencil of.
	 * \param color color to clear the framebuffer to.
	 * \param stencilValue value to fill the stencil buffer with.
	 *
	 * \sa clearFramebufferDepth()
	 * \sa clearFramebufferStencil()
	 * \sa clearFramebufferDepthAndStencil()
	 * \sa clearFramebufferColor()
	 * \sa clearFramebufferColorAndDepth()
	 * \sa clearFramebufferColorAndDepthAndStencil()
	 */
	void clearFramebufferColorAndStencil(Framebuffer& framebuffer, Color color, std::int32_t stencilValue);

	/**
	 * Clear the color, depth and stencil buffer contents of a Framebuffer.
	 *
	 * \param framebuffer the framebuffer to clear the color, depth and stencil
	 *        of.
	 * \param color color to clear the framebuffer to.
	 * \param stencilValue value to fill the stencil buffer with.
	 *
	 * \sa clearFramebufferDepth()
	 * \sa clearFramebufferStencil()
	 * \sa clearFramebufferDepthAndStencil()
	 * \sa clearFramebufferColor()
	 * \sa clearFramebufferColorAndDepth()
	 * \sa clearFramebufferColorAndStencil()
	 */
	void clearFramebufferColorAndDepthAndStencil(Framebuffer& framebuffer, Color color, std::int32_t stencilValue);

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
	static constexpr std::array<std::byte, 4> WHITE_PIXEL_RGBA8{std::byte{255}, std::byte{255}, std::byte{255}, std::byte{255}};
	static constexpr std::array<std::byte, 4> GRAY_PIXEL_RGBA8{std::byte{128}, std::byte{128}, std::byte{128}, std::byte{255}};
	static constexpr std::array<std::byte, 3> NORMAL_PIXEL_RGB8{std::byte{128}, std::byte{128}, std::byte{255}};
	static constexpr TextureOptions PIXEL_TEXTURE_OPTIONS{.repeat = true, .useLinearFiltering = false, .useMipmap = false};

	Shader2D defaultShader;
	Shader2D glyphShader;
	Shader3D modelShader;
	Texture whiteTexture{TextureInternalFormat::RGBA8, 1, 1, TextureFormat::RGBA, TextureComponentType::U8, WHITE_PIXEL_RGBA8.data(), PIXEL_TEXTURE_OPTIONS};
	Texture grayTexture{TextureInternalFormat::RGBA8, 1, 1, TextureFormat::RGBA, TextureComponentType::U8, GRAY_PIXEL_RGBA8.data(), PIXEL_TEXTURE_OPTIONS};
	Texture normalTexture{TextureInternalFormat::RGB8, 1, 1, TextureFormat::RGB, TextureComponentType::U8, NORMAL_PIXEL_RGB8.data(), PIXEL_TEXTURE_OPTIONS};
	TexturedQuad texturedQuad{};
};

} // namespace graphics
} // namespace donut

#endif
