#ifndef DONUT_GRAPHICS_SHADER_STAGE_HPP
#define DONUT_GRAPHICS_SHADER_STAGE_HPP

#include <donut/UniqueHandle.hpp>
#include <donut/graphics/Handle.hpp>

namespace donut {
namespace graphics {

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

	UniqueHandle<Handle, ShaderDeleter> shader{};
};

} // namespace graphics
} // namespace donut

#endif
