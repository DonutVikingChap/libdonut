#include <donut/graphics/Error.hpp>
#include <donut/graphics/Handle.hpp>
#include <donut/graphics/ShaderParameter.hpp>
#include <donut/graphics/ShaderProgram.hpp>
#include <donut/graphics/opengl.hpp>

#include <cstddef> // std::size_t
#include <string>  // std::string

namespace donut {
namespace graphics {

ShaderProgram::ShaderProgram(const ShaderProgramOptions& options)
	: vertexShader(ShaderStageType::VERTEX_SHADER, options.definitions, options.vertexShaderSourceCode)
	, fragmentShader(ShaderStageType::FRAGMENT_SHADER, options.definitions, options.fragmentShaderSourceCode) {
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

void ShaderProgram::setUniformFloat(const ShaderParameter& parameter, float value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformVec2(const ShaderParameter& parameter, glm::vec2 value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformVec3(const ShaderParameter& parameter, glm::vec3 value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformVec4(const ShaderParameter& parameter, glm::vec4 value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformInt(const ShaderParameter& parameter, glm::i32 value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformIVec2(const ShaderParameter& parameter, glm::i32vec2 value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformIVec3(const ShaderParameter& parameter, glm::i32vec3 value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformIVec4(const ShaderParameter& parameter, glm::i32vec4 value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformUint(const ShaderParameter& parameter, glm::u32 value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformUVec2(const ShaderParameter& parameter, glm::u32vec2 value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformUVec3(const ShaderParameter& parameter, glm::u32vec3 value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformUVec4(const ShaderParameter& parameter, glm::u32vec4 value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformMat2(const ShaderParameter& parameter, const glm::mat2& value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformMat3(const ShaderParameter& parameter, const glm::mat3& value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformMat4(const ShaderParameter& parameter, const glm::mat4& value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::ProgramDeleter::operator()(Handle handle) const noexcept {
	glDeleteProgram(handle);
}

} // namespace graphics
} // namespace donut
