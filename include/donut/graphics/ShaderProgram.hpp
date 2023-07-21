#ifndef DONUT_GRAPHICS_SHADER_PROGRAM_HPP
#define DONUT_GRAPHICS_SHADER_PROGRAM_HPP

#include <donut/UniqueHandle.hpp>
#include <donut/Variant.hpp>
#include <donut/graphics/Handle.hpp>
#include <donut/graphics/ShaderStage.hpp>

#include <cstdint>     // std::int32_t
#include <glm/glm.hpp> // glm::...
#include <span>        // std::span
#include <utility>     // std::pair
#include <vector>      // std::vector

namespace donut {
namespace graphics {

class ShaderParameter; // Forward declaration, to avoid a circular include of ShaderParameter.hpp.

/**
 * Configuration options for a ShaderProgram.
 */
struct ShaderProgramOptions {
	/**
	 * Non-owning pointer to GLSL source code for constant definitions to add to
	 * the beginning of the source code, or nullptr to not add any definitions.
	 */
	const char* definitions = nullptr;

	/**
	 * Non-owning pointer to the GLSL source code for the vertex shader stage, or
	 * nullptr to use the built-in vertex pipeline.
	 */
	const char* vertexShaderSourceCode = nullptr;

	/**
	 * Non-owning pointer to the GLSL source code for the fragment shader stage,
	 * or nullptr to use the built-in fragment pipeline.
	 */
	const char* fragmentShaderSourceCode = nullptr;
};

/**
 * Compiled and linked GPU shader program.
 */
class ShaderProgram {
public:
	/**
	 * Value of a uniform shader variable.
	 */
	using UniformValue = Variant< //
		float,                    //
		glm::vec2,                //
		glm::vec3,                //
		glm::vec4,                //
		glm::i32,                 //
		glm::i32vec2,             //
		glm::i32vec3,             //
		glm::i32vec4,             //
		glm::u32,                 //
		glm::u32vec2,             //
		glm::u32vec3,             //
		glm::u32vec4,             //
		glm::mat2,                //
		glm::mat3,                //
		glm::mat4>;

	/**
	 * Compile and link a shader program.
	 *
	 * \param options shader program options, see ShaderProgramOptions.
	 *
	 * \throws graphics::Error on failure to create a shader object, create the
	 *         shader program object, compile the shader code or link the shader
	 *         program.
	 * \throws std::bad_alloc on allocation failure.
	 */
	explicit ShaderProgram(const ShaderProgramOptions& options);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * float.
	 *
	 * \param parameter shader variable to set.
	 * \param value new value to set the variable to.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note The new value is not uploaded to the shader immediately; instead it
	 *       is stored in this shader program's uniform upload queue to be
	 *       uploaded on the next render that uses this shader.
	 *
	 * \sa getUniformUploadQueue()
	 * \sa clearUniformUploadQueue()
	 */
	void setUniformFloat(const ShaderParameter& parameter, float value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * vec2.
	 *
	 * \param parameter shader variable to set.
	 * \param value new value to set the variable to.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note The new value is not uploaded to the shader immediately; instead it
	 *       is stored in this shader program's uniform upload queue to be
	 *       uploaded on the next render that uses this shader.
	 *
	 * \sa getUniformUploadQueue()
	 * \sa clearUniformUploadQueue()
	 */
	void setUniformVec2(const ShaderParameter& parameter, glm::vec2 value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * vec3.
	 *
	 * \param parameter shader variable to set.
	 * \param value new value to set the variable to.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note The new value is not uploaded to the shader immediately; instead it
	 *       is stored in this shader program's uniform upload queue to be
	 *       uploaded on the next render that uses this shader.
	 *
	 * \sa getUniformUploadQueue()
	 * \sa clearUniformUploadQueue()
	 */
	void setUniformVec3(const ShaderParameter& parameter, glm::vec3 value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * vec4.
	 *
	 * \param parameter shader variable to set.
	 * \param value new value to set the variable to.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note The new value is not uploaded to the shader immediately; instead it
	 *       is stored in this shader program's uniform upload queue to be
	 *       uploaded on the next render that uses this shader.
	 *
	 * \sa getUniformUploadQueue()
	 * \sa clearUniformUploadQueue()
	 */
	void setUniformVec4(const ShaderParameter& parameter, glm::vec4 value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * int.
	 *
	 * \param parameter shader variable to set.
	 * \param value new value to set the variable to.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note The new value is not uploaded to the shader immediately; instead it
	 *       is stored in this shader program's uniform upload queue to be
	 *       uploaded on the next render that uses this shader.
	 *
	 * \sa getUniformUploadQueue()
	 * \sa clearUniformUploadQueue()
	 */
	void setUniformInt(const ShaderParameter& parameter, glm::i32 value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * ivec2.
	 *
	 * \param parameter shader variable to set.
	 * \param value new value to set the variable to.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note The new value is not uploaded to the shader immediately; instead it
	 *       is stored in this shader program's uniform upload queue to be
	 *       uploaded on the next render that uses this shader.
	 *
	 * \sa getUniformUploadQueue()
	 * \sa clearUniformUploadQueue()
	 */
	void setUniformIVec2(const ShaderParameter& parameter, glm::i32vec2 value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * ivec3.
	 *
	 * \param parameter shader variable to set.
	 * \param value new value to set the variable to.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note The new value is not uploaded to the shader immediately; instead it
	 *       is stored in this shader program's uniform upload queue to be
	 *       uploaded on the next render that uses this shader.
	 *
	 * \sa getUniformUploadQueue()
	 * \sa clearUniformUploadQueue()
	 */
	void setUniformIVec3(const ShaderParameter& parameter, glm::i32vec3 value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * ivec4.
	 *
	 * \param parameter shader variable to set.
	 * \param value new value to set the variable to.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note The new value is not uploaded to the shader immediately; instead it
	 *       is stored in this shader program's uniform upload queue to be
	 *       uploaded on the next render that uses this shader.
	 *
	 * \sa getUniformUploadQueue()
	 * \sa clearUniformUploadQueue()
	 */
	void setUniformIVec4(const ShaderParameter& parameter, glm::i32vec4 value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * uint.
	 *
	 * \param parameter shader variable to set.
	 * \param value new value to set the variable to.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note The new value is not uploaded to the shader immediately; instead it
	 *       is stored in this shader program's uniform upload queue to be
	 *       uploaded on the next render that uses this shader.
	 *
	 * \sa getUniformUploadQueue()
	 * \sa clearUniformUploadQueue()
	 */
	void setUniformUint(const ShaderParameter& parameter, glm::u32 value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * uvec2.
	 *
	 * \param parameter shader variable to set.
	 * \param value new value to set the variable to.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note The new value is not uploaded to the shader immediately; instead it
	 *       is stored in this shader program's uniform upload queue to be
	 *       uploaded on the next render that uses this shader.
	 *
	 * \sa getUniformUploadQueue()
	 * \sa clearUniformUploadQueue()
	 */
	void setUniformUVec2(const ShaderParameter& parameter, glm::u32vec2 value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * uvec3.
	 *
	 * \param parameter shader variable to set.
	 * \param value new value to set the variable to.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note The new value is not uploaded to the shader immediately; instead it
	 *       is stored in this shader program's uniform upload queue to be
	 *       uploaded on the next render that uses this shader.
	 *
	 * \sa getUniformUploadQueue()
	 * \sa clearUniformUploadQueue()
	 */
	void setUniformUVec3(const ShaderParameter& parameter, glm::u32vec3 value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * uvec4.
	 *
	 * \param parameter shader variable to set.
	 * \param value new value to set the variable to.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note The new value is not uploaded to the shader immediately; instead it
	 *       is stored in this shader program's uniform upload queue to be
	 *       uploaded on the next render that uses this shader.
	 *
	 * \sa getUniformUploadQueue()
	 * \sa clearUniformUploadQueue()
	 */
	void setUniformUVec4(const ShaderParameter& parameter, glm::u32vec4 value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * mat2.
	 *
	 * \param parameter shader variable to set.
	 * \param value new value to set the variable to.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note The new value is not uploaded to the shader immediately; instead it
	 *       is stored in this shader program's uniform upload queue to be
	 *       uploaded on the next render that uses this shader.
	 *
	 * \sa getUniformUploadQueue()
	 * \sa clearUniformUploadQueue()
	 */
	void setUniformMat2(const ShaderParameter& parameter, const glm::mat2& value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * mat3.
	 *
	 * \param parameter shader variable to set.
	 * \param value new value to set the variable to.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note The new value is not uploaded to the shader immediately; instead it
	 *       is stored in this shader program's uniform upload queue to be
	 *       uploaded on the next render that uses this shader.
	 *
	 * \sa getUniformUploadQueue()
	 * \sa clearUniformUploadQueue()
	 */
	void setUniformMat3(const ShaderParameter& parameter, const glm::mat3& value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * mat4.
	 *
	 * \param parameter shader variable to set.
	 * \param value new value to set the variable to.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note The new value is not uploaded to the shader immediately; instead it
	 *       is stored in this shader program's uniform upload queue to be
	 *       uploaded on the next render that uses this shader.
	 *
	 * \sa getUniformUploadQueue()
	 * \sa clearUniformUploadQueue()
	 */
	void setUniformMat4(const ShaderParameter& parameter, const glm::mat4& value);

	/**
	 * Erase all entries from the queue of new uniform shader variable values.
	 */
	void clearUniformUploadQueue() noexcept {
		uniformUploadQueue.clear();
	}

	/**
	 * Get the list of new uniform shader variable values to be uploaded to the
	 * shader.
	 *
	 * \return a non-owning read-only view over a sequence of pairs where the
	 *         first element of each pair represents the location of a uniform
	 *         shader variable and the second element holds its new value.
	 */
	[[nodiscard]] std::span<const std::pair<std::int32_t, UniformValue>> getUniformUploadQueue() const noexcept {
		return uniformUploadQueue;
	}

	/**
	 * Get an opaque handle to the GPU representation of the shader program.
	 *
	 * \return a non-owning resource handle to the GPU representation of the
	 *         shader program.
	 *
	 * \note This function is used internally by the implementations of various
	 *       abstractions and is not intended to be used outside of the graphics
	 *       module. The returned handle has no meaning to application code.
	 */
	[[nodiscard]] Handle get() const noexcept {
		return program.get();
	}

private:
	struct ProgramDeleter {
		void operator()(Handle handle) const noexcept;
	};

	UniqueHandle<Handle, ProgramDeleter> program{};
	ShaderStage vertexShader;
	ShaderStage fragmentShader;
	std::vector<std::pair<std::int32_t, UniformValue>> uniformUploadQueue{};
};

} // namespace graphics
} // namespace donut

#endif
