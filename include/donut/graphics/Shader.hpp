#ifndef DONUT_GRAPHICS_SHADER_HPP
#define DONUT_GRAPHICS_SHADER_HPP

#include <donut/Resource.hpp>
#include <donut/Variant.hpp>
#include <donut/graphics/Handle.hpp>

#include <array>       // std::array
#include <cstddef>     // std::size_t
#include <cstdint>     // std::int32_t
#include <glm/glm.hpp> // glm::...
#include <span>        // std::span
#include <string>      // std::string, std::to_string
#include <utility>     // std::pair, std::index_sequence, std::make_index_sequence
#include <vector>      // std::vector

namespace donut {
namespace graphics {

class ShaderUniform; // Forward declaration of the definition below, so that ShaderProgram can take references to it without a circular definition.

/**
 * Type of a ShaderStage in a ShaderProgram.
 */
enum class ShaderStageType : unsigned {
	VERTEX_SHADER = 0x8B31,   ///< Vertex shader. \hideinitializer
	FRAGMENT_SHADER = 0x8B30, ///< Fragment/pixel shader. \hideinitializer
};

/**
 * Compiled GPU code for a specific stage of a ShaderProgram.
 */
class ShaderStage {
public:
	/**
	 * Construct an empty shader stage without a value.
	 */
	constexpr explicit ShaderStage() noexcept = default;

	/**
	 * Compile a shader stage.
	 *
	 * \param type type of shader stage, see ShaderStageType.
	 * \param definitions non-owning pointer to the GLSL source code for
	 *        constant definitions to add to the beginning of the source code,
	 *        or nullptr to not add any definitions.
	 * \param sourceCode non-owning pointer to the GLSL source code to compile,
	 *        or nullptr to create an empty shader stage without a value.
	 *
	 * \throws graphics::Error on failure to create the shader object or compile
	 *         the shader code.
	 * \throws std::bad_alloc on allocation failure.
	 */
	explicit ShaderStage(ShaderStageType type, const char* definitions, const char* sourceCode);

	/**
	 * Check if the shader stage has a value.
	 *
	 * \return true if the shader stage has a value, false otherwise.
	 */
	explicit operator bool() const noexcept {
		return static_cast<bool>(shader);
	}

	/**
	 * Get an opaque handle to the GPU representation of the shader stage.
	 *
	 * \return a non-owning resource handle to the GPU representation of the
	 *         shader stage.
	 *
	 * \note This function is used internally by the implementations of various
	 *       abstractions and is not intended to be used outside of the graphics
	 *       module. The returned handle has no meaning to application code.
	 */
	[[nodiscard]] Handle get() const noexcept {
		return shader.get();
	}

private:
	struct ShaderDeleter {
		void operator()(Handle handle) const noexcept;
	};

	Resource<Handle, ShaderDeleter> shader{};
};

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
	 * \param uniform shader variable to set.
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
	void setUniformFloat(const ShaderUniform& uniform, float value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * vec2.
	 *
	 * \param uniform shader variable to set.
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
	void setUniformVec2(const ShaderUniform& uniform, glm::vec2 value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * vec3.
	 *
	 * \param uniform shader variable to set.
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
	void setUniformVec3(const ShaderUniform& uniform, glm::vec3 value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * vec4.
	 *
	 * \param uniform shader variable to set.
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
	void setUniformVec4(const ShaderUniform& uniform, glm::vec4 value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * int.
	 *
	 * \param uniform shader variable to set.
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
	void setUniformInt(const ShaderUniform& uniform, glm::i32 value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * ivec2.
	 *
	 * \param uniform shader variable to set.
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
	void setUniformIVec2(const ShaderUniform& uniform, glm::i32vec2 value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * ivec3.
	 *
	 * \param uniform shader variable to set.
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
	void setUniformIVec3(const ShaderUniform& uniform, glm::i32vec3 value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * ivec4.
	 *
	 * \param uniform shader variable to set.
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
	void setUniformIVec4(const ShaderUniform& uniform, glm::i32vec4 value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * uint.
	 *
	 * \param uniform shader variable to set.
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
	void setUniformUint(const ShaderUniform& uniform, glm::u32 value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * uvec2.
	 *
	 * \param uniform shader variable to set.
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
	void setUniformUVec2(const ShaderUniform& uniform, glm::u32vec2 value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * uvec3.
	 *
	 * \param uniform shader variable to set.
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
	void setUniformUVec3(const ShaderUniform& uniform, glm::u32vec3 value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * uvec4.
	 *
	 * \param uniform shader variable to set.
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
	void setUniformUVec4(const ShaderUniform& uniform, glm::u32vec4 value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * mat2.
	 *
	 * \param uniform shader variable to set.
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
	void setUniformMat2(const ShaderUniform& uniform, const glm::mat2& value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * mat3.
	 *
	 * \param uniform shader variable to set.
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
	void setUniformMat3(const ShaderUniform& uniform, const glm::mat3& value);

	/**
	 * Enqueue a new value to be set for a uniform shader variable of GLSL type
	 * mat4.
	 *
	 * \param uniform shader variable to set.
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
	void setUniformMat4(const ShaderUniform& uniform, const glm::mat4& value);

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

	Resource<Handle, ProgramDeleter> program{};
	ShaderStage vertexShader;
	ShaderStage fragmentShader;
	std::vector<std::pair<std::int32_t, UniformValue>> uniformUploadQueue{};
};

/**
 * Identifier for a uniform shader variable inside a ShaderProgram.
 */
class ShaderUniform {
public:
	/**
	 * Construct an identifier for a specific uniform shader variable.
	 *
	 * \param program shader program in which the variable resides.
	 * \param name name of the variable.
	 *
	 * \note If the variable is not found, the resulting identifier will be
	 *       invalid.
	 */
	ShaderUniform(const ShaderProgram& program, const char* name);

	/**
	 * Get the location of the variable in the shader program.
	 *
	 * \return The location of the variable, or -1 if the identifier is invalid.
	 */
	[[nodiscard]] std::int32_t getLocation() const noexcept {
		return location;
	}

private:
	std::int32_t location;
};

/**
 * Fixed-size array of uniform shader variable identifiers representing an array
 * inside a ShaderProgram.
 *
 * \tparam T the type of uniform shader variable identifier to use for each
 *         element of the array.
 * \tparam N the number of elements in the array.
 */
template <typename T, std::size_t N>
class ShaderArray {
public:
	/**
	 * Construct an array of uniform shader variable identifiers for a specific
	 * shader array.
	 *
	 * \param program shader program in which the array resides.
	 * \param name name of the array.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note If the array elements are not found, the resulting identifiers will
	 *       be invalid.
	 */
	ShaderArray(const ShaderProgram& program, const char* name)
		: array([&]<std::size_t... Indices>(std::index_sequence<Indices...>)->std::array<T, N> {
			return {(T{program, (std::string{name} + "[" + std::to_string(Indices) + "]").c_str()})...};
		}(std::make_index_sequence<N>{})) {}

	/**
	 * Get the size of the array.
	 *
	 * \return the number of elements in the array, which is always equal to
	 *         template parameter N.
	 */
	[[nodiscard]] std::size_t size() const noexcept {
		return array.size();
	}

	/**
	 * Access an element of the array.
	 *
	 * \param i the index of the element to get. Must be less than the size of
	 *        the array.
	 *
	 * \return a reference to the element at index i.
	 */
	[[nodiscard]] T& operator[](std::size_t i) {
		return array[i];
	}

	/**
	 * Access an element of the array.
	 *
	 * \param i the index of the element to get. Must be less than the size of
	 *        the array.
	 *
	 * \return a read-only reference to the element at index i.
	 */
	[[nodiscard]] const T& operator[](std::size_t i) const {
		return array[i];
	}

	/**
	 * Get an iterator to the beginning of the array.
	 *
	 * \return the begin iterator.
	 */
	[[nodiscard]] decltype(auto) begin() const noexcept {
		return array.begin();
	}

	/**
	 * Get an iterator to the end of the array.
	 *
	 * \return the end iterator.
	 */
	[[nodiscard]] decltype(auto) end() const noexcept {
		return array.end();
	}

private:
	std::array<T, N> array;
};

} // namespace graphics
} // namespace donut

#endif
