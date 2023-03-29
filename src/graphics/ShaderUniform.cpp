#include <donut/graphics/ShaderProgram.hpp>
#include <donut/graphics/ShaderUniform.hpp>
#include <donut/graphics/opengl.hpp>

namespace donut {
namespace graphics {

ShaderUniform::ShaderUniform(const ShaderProgram& program, const char* name)
	: location(glGetUniformLocation(program.get(), name)) {}

} // namespace graphics
} // namespace donut
