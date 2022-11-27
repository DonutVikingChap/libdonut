#ifndef DONUT_GRAPHICS_SHADER_3D_HPP
#define DONUT_GRAPHICS_SHADER_3D_HPP

#include <donut/graphics/Shader.hpp>

namespace donut {
namespace graphics {

struct Shader3DOptions {
	bool clearDepthBuffer = true;
	bool overwriteDepthBuffer = false;
	bool useDepthTest = true;
	bool useBackfaceCulling = true;
	bool useAlphaBlending = false;
};

class Shader3D {
public:
	static const std::string_view vertexShaderSourceCodeInstancedModel;
	static const std::string_view fragmentShaderSourceCodeModelBlinnPhong;

	Shader3D(std::string_view vertexShaderSourceCode, std::string_view fragmentShaderSourceCode, const Shader3DOptions& options = {})
		: options(options)
		, program({
			  .vertexShaderSourceCode = vertexShaderSourceCode,
			  .fragmentShaderSourceCode = fragmentShaderSourceCode,
		  }) {}

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
