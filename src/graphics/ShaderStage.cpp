#include <donut/graphics/Error.hpp>
#include <donut/graphics/Handle.hpp>
#include <donut/graphics/ShaderStage.hpp>
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

} // namespace graphics
} // namespace donut
