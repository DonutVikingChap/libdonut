#ifndef DONUT_GRAPHICS_SHADER_2D_HPP
#define DONUT_GRAPHICS_SHADER_2D_HPP

#include <donut/graphics/Shader.hpp>

#include <string_view> // std::string_view

namespace donut {
namespace graphics {

struct Shader2DOptions {
	bool clearDepthBuffer = false;
	bool overwriteDepthBuffer = false;
	bool useDepthTest = false;
	bool useBackfaceCulling = false;
	bool useAlphaBlending = true;
};

class Shader2D {
public:
	static const std::string_view vertexShaderSourceCodeInstancedTexturedQuad;
	static const std::string_view fragmentShaderSourceCodeTexturedQuadPlain;
	static const std::string_view fragmentShaderSourceCodeTexturedQuadAlpha;

	Shader2D(std::string_view vertexShaderSourceCode, std::string_view fragmentShaderSourceCode, const Shader2DOptions& options = {})
		: options(options)
		, program({
			  .vertexShaderSourceCode = vertexShaderSourceCode,
			  .fragmentShaderSourceCode = fragmentShaderSourceCode,
		  }) {}

	Shader2DOptions options;
	ShaderProgram program;
	ShaderUniform projectionViewMatrix{program, "projectionViewMatrix"};
	ShaderUniform textureUnit{program, "textureUnit"};
};

} // namespace graphics
} // namespace donut

#endif
