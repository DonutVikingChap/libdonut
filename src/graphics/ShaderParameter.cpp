#include <donut/graphics/ShaderParameter.hpp>
#include <donut/graphics/ShaderProgram.hpp>
#include <donut/graphics/opengl.hpp>

namespace donut {
namespace graphics {

ShaderParameter::ShaderParameter(const ShaderProgram& program, const char* name)
	: location(glGetUniformLocation(program.get(), name)) {}

} // namespace graphics
} // namespace donut