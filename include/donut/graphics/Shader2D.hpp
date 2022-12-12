#ifndef DONUT_GRAPHICS_SHADER_2D_HPP
#define DONUT_GRAPHICS_SHADER_2D_HPP

#include <donut/graphics/Shader.hpp>

namespace donut {
namespace graphics {

struct Shader2DOptions {
	int orderIndex = 1;
	bool clearDepthBuffer = false;
	bool overwriteDepthBuffer = false;
	bool useDepthTest = false;
	bool useBackfaceCulling = false;
	bool useAlphaBlending = true;
};

class Shader2D {
public:
	static const char* const vertexShaderSourceCodeInstancedTexturedQuad;
	static const char* const fragmentShaderSourceCodeTexturedQuadPlain;
	static const char* const fragmentShaderSourceCodeTexturedQuadAlpha;

	Shader2D(const ShaderProgramOptions& programOptions, const Shader2DOptions& options = {})
		: options(options)
		, program(programOptions) {}

	Shader2DOptions options;
	ShaderProgram program;
	ShaderUniform projectionViewMatrix{program, "projectionViewMatrix"};
	ShaderUniform textureUnit{program, "textureUnit"};
};

} // namespace graphics
} // namespace donut

#endif
