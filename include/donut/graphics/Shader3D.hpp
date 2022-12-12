#ifndef DONUT_GRAPHICS_SHADER_3D_HPP
#define DONUT_GRAPHICS_SHADER_3D_HPP

#include <donut/graphics/Shader.hpp>

namespace donut {
namespace graphics {

struct Shader3DOptions {
	int orderIndex = 1;
	bool clearDepthBuffer = false;
	bool overwriteDepthBuffer = false;
	bool useDepthTest = true;
	bool useBackfaceCulling = true;
	bool useAlphaBlending = false;
};

class Shader3D {
public:
	static const char* const vertexShaderSourceCodeInstancedModel;
	static const char* const fragmentShaderSourceCodeModelBlinnPhong;

	Shader3D(const ShaderProgramOptions& programOptions, const Shader3DOptions& options = {})
		: options(options)
		, program(programOptions) {}

	Shader3DOptions options;
	ShaderProgram program;
	ShaderUniform projectionViewMatrix{program, "projectionViewMatrix"};
	ShaderUniform diffuseMap{program, "diffuseMap"};
	ShaderUniform specularMap{program, "specularMap"};
	ShaderUniform normalMap{program, "normalMap"};
	ShaderUniform specularExponent{program, "specularExponent"};
};

} // namespace graphics
} // namespace donut

#endif
