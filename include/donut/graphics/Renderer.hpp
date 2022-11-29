#ifndef DONUT_GRAPHICS_RENDERER_HPP
#define DONUT_GRAPHICS_RENDERER_HPP

#include <donut/graphics/Framebuffer.hpp>
#include <donut/graphics/RenderPass.hpp>
#include <donut/graphics/Shader2D.hpp>
#include <donut/graphics/Shader3D.hpp>
#include <donut/graphics/Texture.hpp>
#include <donut/graphics/TexturedQuad.hpp>
#include <donut/graphics/Viewport.hpp>

#include <array>
#include <glm/glm.hpp>
#include <string_view>

namespace donut {
namespace graphics {

class Renderer;

struct RendererOptions {
	std::string_view defaultVertexShaderSourceCode = Shader2D::vertexShaderSourceCodeInstancedTexturedQuad;
	std::string_view defaultFragmentShaderSourceCode = Shader2D::fragmentShaderSourceCodeTexturedQuadPlain;
	Shader2DOptions defaultShaderOptions{};
	std::string_view glyphVertexShaderSourceCode = Shader2D::vertexShaderSourceCodeInstancedTexturedQuad;
	std::string_view glyphFragmentShaderSourceCode = Shader2D::fragmentShaderSourceCodeTexturedQuadAlpha;
	Shader2DOptions glyphShaderOptions{};
	std::string_view modelVertexShaderSourceCode = Shader3D::vertexShaderSourceCodeInstancedModel;
	std::string_view modelFragmentShaderSourceCode = Shader3D::fragmentShaderSourceCodeModelBlinnPhong;
	Shader3DOptions modelShaderOptions{};
};

class Renderer {
public:
	explicit Renderer(const RendererOptions& options = {})
		: defaultShader(options.defaultVertexShaderSourceCode, options.defaultFragmentShaderSourceCode, options.defaultShaderOptions)
		, glyphShader(options.glyphVertexShaderSourceCode, options.glyphFragmentShaderSourceCode, options.glyphShaderOptions)
		, modelShader(options.modelVertexShaderSourceCode, options.modelFragmentShaderSourceCode, options.modelShaderOptions) {}

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
