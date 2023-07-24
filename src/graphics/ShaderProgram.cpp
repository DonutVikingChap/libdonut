#include <donut/graphics/Error.hpp>
#include <donut/graphics/Handle.hpp>
#include <donut/graphics/ShaderParameter.hpp>
#include <donut/graphics/ShaderProgram.hpp>
#include <donut/graphics/opengl.hpp>

#include <algorithm> // std::find_if
#include <cstddef>   // std::size_t
#include <string>    // std::string

namespace donut::graphics {

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

void ShaderProgram::setUniformVec2(const ShaderParameter& parameter, vec2 value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformVec3(const ShaderParameter& parameter, vec3 value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformVec4(const ShaderParameter& parameter, vec4 value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformInt(const ShaderParameter& parameter, i32 value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformIVec2(const ShaderParameter& parameter, i32vec2 value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformIVec3(const ShaderParameter& parameter, i32vec3 value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformIVec4(const ShaderParameter& parameter, i32vec4 value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformUint(const ShaderParameter& parameter, u32 value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformUVec2(const ShaderParameter& parameter, u32vec2 value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformUVec3(const ShaderParameter& parameter, u32vec3 value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformUVec4(const ShaderParameter& parameter, u32vec4 value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformMat2(const ShaderParameter& parameter, const mat2& value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformMat3(const ShaderParameter& parameter, const mat3& value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformMat4(const ShaderParameter& parameter, const mat4& value) {
	uniformUploadQueue.emplace_back(parameter.getLocation(), value);
}

void ShaderProgram::setUniformSampler(const ShaderParameter& parameter, const Texture* texture) {
	const std::int32_t location = parameter.getLocation();
	if (const auto it =
			std::find_if(textureBindings.begin(), textureBindings.end(), [location](const std::pair<std::int32_t, const Texture*>& kv) -> bool { return kv.first == location; });
		it != textureBindings.end()) {
		if (texture) {
			it->second = texture;
		} else {
			textureBindings.erase(it);
		}
	} else if (texture) {
		textureBindings.emplace_back(location, texture);
	}
}

void ShaderProgram::ProgramDeleter::operator()(Handle handle) const noexcept {
	glDeleteProgram(handle);
}

} // namespace donut::graphics
