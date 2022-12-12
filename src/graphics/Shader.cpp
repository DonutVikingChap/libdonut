#include <donut/graphics/Error.hpp>
#include <donut/graphics/Handle.hpp>
#include <donut/graphics/Shader.hpp>
#include <donut/graphics/opengl.hpp>

#include <array>   // std::array
#include <cstddef> // std::size_t
#include <string>  // std::string

namespace donut {
namespace graphics {

ShaderStage::ShaderStage(ShaderStageType type, const char* definitions, const char* sourceCode) {
	if (!sourceCode) {
		return;
	}

	if (!definitions) {
		definitions = "";
	}

	const Handle handle = glCreateShader(static_cast<GLenum>(type));
	if (!handle) {
		throw Error{"Failed to create shader object!"};
	}
	shader.reset(handle);

#ifdef __EMSCRIPTEN__
	constexpr const char* HEADER = "#version 300 es\nprecision highp float;\nprecision lowp sampler2DArray;\n";
#else
	constexpr const char* HEADER = "#version 330 core\n";
#endif
	constexpr const char* LINE_DIRECTIVE = "\n#line 1\n";

	const std::array<const GLchar*, 4> sourceStrings{
		static_cast<const GLchar*>(HEADER),
		static_cast<const GLchar*>(definitions),
		static_cast<const GLchar*>(LINE_DIRECTIVE),
		static_cast<const GLchar*>(sourceCode),
	};
	glShaderSource(shader.get(), sourceStrings.size(), sourceStrings.data(), nullptr);
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
	: vertexShader(ShaderStageType::VERTEX_SHADER, options.definitions, options.vertexShaderSourceCode)
	, fragmentShader(ShaderStageType::FRAGMENT_SHADER, options.definitions, options.fragmentShaderSourceCode) {
	if (!options.vertexShaderSourceCode && !options.fragmentShaderSourceCode) {
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

void ShaderProgram::setUniformFloat(const ShaderUniform& uniform, float value) {
	uniformUploadQueue.emplace_back(uniform.getLocation(), value);
}

void ShaderProgram::setUniformVec2(const ShaderUniform& uniform, glm::vec2 value) {
	uniformUploadQueue.emplace_back(uniform.getLocation(), value);
}

void ShaderProgram::setUniformVec3(const ShaderUniform& uniform, glm::vec3 value) {
	uniformUploadQueue.emplace_back(uniform.getLocation(), value);
}

void ShaderProgram::setUniformVec4(const ShaderUniform& uniform, glm::vec4 value) {
	uniformUploadQueue.emplace_back(uniform.getLocation(), value);
}

void ShaderProgram::setUniformInt(const ShaderUniform& uniform, std::int32_t value) {
	uniformUploadQueue.emplace_back(uniform.getLocation(), value);
}

void ShaderProgram::setUniformIVec2(const ShaderUniform& uniform, glm::i32vec2 value) {
	uniformUploadQueue.emplace_back(uniform.getLocation(), value);
}

void ShaderProgram::setUniformIVec3(const ShaderUniform& uniform, glm::i32vec3 value) {
	uniformUploadQueue.emplace_back(uniform.getLocation(), value);
}

void ShaderProgram::setUniformIVec4(const ShaderUniform& uniform, glm::i32vec4 value) {
	uniformUploadQueue.emplace_back(uniform.getLocation(), value);
}

void ShaderProgram::setUniformUint(const ShaderUniform& uniform, glm::uint32_t value) {
	uniformUploadQueue.emplace_back(uniform.getLocation(), value);
}

void ShaderProgram::setUniformUVec2(const ShaderUniform& uniform, glm::u32vec2 value) {
	uniformUploadQueue.emplace_back(uniform.getLocation(), value);
}

void ShaderProgram::setUniformUVec3(const ShaderUniform& uniform, glm::u32vec3 value) {
	uniformUploadQueue.emplace_back(uniform.getLocation(), value);
}

void ShaderProgram::setUniformUVec4(const ShaderUniform& uniform, glm::u32vec4 value) {
	uniformUploadQueue.emplace_back(uniform.getLocation(), value);
}

void ShaderProgram::setUniformMat2(const ShaderUniform& uniform, const glm::mat2& value) {
	uniformUploadQueue.emplace_back(uniform.getLocation(), value);
}

void ShaderProgram::setUniformMat3(const ShaderUniform& uniform, const glm::mat3& value) {
	uniformUploadQueue.emplace_back(uniform.getLocation(), value);
}

void ShaderProgram::setUniformMat4(const ShaderUniform& uniform, const glm::mat4& value) {
	uniformUploadQueue.emplace_back(uniform.getLocation(), value);
}

void ShaderProgram::ProgramDeleter::operator()(Handle handle) const noexcept {
	glDeleteProgram(handle);
}

ShaderUniform::ShaderUniform(const ShaderProgram& program, const char* name)
	: location((program) ? glGetUniformLocation(program.get(), name) : -1) {}

} // namespace graphics
} // namespace donut
