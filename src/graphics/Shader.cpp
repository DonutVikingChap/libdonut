#include <donut/graphics/Error.hpp>
#include <donut/graphics/Handle.hpp>
#include <donut/graphics/Shader.hpp>
#include <donut/graphics/opengl.hpp>

#include <array>
#include <exception>
#include <string>
#include <string_view>

namespace donut {
namespace graphics {

ShaderStage::ShaderStage(ShaderStageType type, std::string_view sourceCode) {
	if (sourceCode.empty()) {
		return;
	}

	const Handle handle = glCreateShader(static_cast<GLenum>(type));
	if (!handle) {
		throw Error{"Failed to create shader object!"};
	}
	shader.reset(handle);

#ifdef __EMSCRIPTEN__
	constexpr std::string_view header{"#version 300 es\nprecision highp float;\nprecision lowp sampler2DArray;\n"};
#else
	constexpr std::string_view header{"#version 330 core\n"};
#endif
	const std::array sourceStrings{static_cast<const GLchar*>(header.data()), static_cast<const GLchar*>(sourceCode.data())};
	const std::array sourceLengths{static_cast<GLint>(header.size()), static_cast<GLint>(sourceCode.size())};
	glShaderSource(shader.get(), sourceStrings.size(), sourceStrings.data(), sourceLengths.data());
	glCompileShader(shader.get());

	GLint success = GL_FALSE;
	glGetShaderiv(shader.get(), GL_COMPILE_STATUS, &success);
	if (success != GL_TRUE) {
		GLint infoLogLength = 0;
		glGetShaderiv(shader.get(), GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0) {
			std::string infoLog(static_cast<std::size_t>(infoLogLength), '\0');
			glGetShaderInfoLog(shader.get(), infoLogLength, nullptr, infoLog.data());
			throw Error{infoLog};
		}
		throw Error{"Failed to compile shader!"};
	}
}

void ShaderStage::ShaderDeleter::operator()(Handle handle) const noexcept {
	glDeleteShader(handle);
}

ShaderProgram::ShaderProgram(const ShaderProgramOptions& options)
	: vertexShader(ShaderStageType::VERTEX_SHADER, options.vertexShaderSourceCode)
	, fragmentShader(ShaderStageType::FRAGMENT_SHADER, options.fragmentShaderSourceCode) {
	if (options.vertexShaderSourceCode.empty() && options.fragmentShaderSourceCode.empty()) {
		return;
	}

	const Handle handle = glCreateProgram();
	if (!handle) {
		throw Error{"Failed to create shader program object!"};
	}
	program.reset(handle);

	if (vertexShader) {
		glAttachShader(program.get(), vertexShader.get());
	}

	if (fragmentShader) {
		glAttachShader(program.get(), fragmentShader.get());
	}

	glLinkProgram(program.get());

	GLint success = GL_FALSE;
	glGetProgramiv(program.get(), GL_LINK_STATUS, &success);
	if (success != GL_TRUE) {
		GLint infoLogLength = 0;
		glGetProgramiv(program.get(), GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0) {
			std::string infoLog(static_cast<std::size_t>(infoLogLength), '\0');
			glGetProgramInfoLog(program.get(), infoLogLength, nullptr, infoLog.data());
			throw Error{infoLog.c_str()};
		}
		throw Error{"Failed to link shader program!"};
	}
}

void ShaderProgram::ProgramDeleter::operator()(Handle handle) const noexcept {
	glDeleteProgram(handle);
}

ShaderUniform::ShaderUniform(const ShaderProgram& program, const char* name)
	: location((program) ? glGetUniformLocation(program.get(), name) : -1) {}

} // namespace graphics
} // namespace donut
