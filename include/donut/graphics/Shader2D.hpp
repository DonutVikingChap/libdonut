#ifndef DONUT_GRAPHICS_SHADER_2D_HPP
#define DONUT_GRAPHICS_SHADER_2D_HPP

#include <donut/graphics/ShaderConfiguration.hpp>
#include <donut/graphics/ShaderProgram.hpp>
#include <donut/graphics/ShaderUniform.hpp>

namespace donut {
namespace graphics {

class Renderer; // Forward declaration, to avoid a circular include of Renderer.hpp.

/**
 * Configuration options for a Shader2D.
 */
struct Shader2DOptions {
	/**
	 * Shader configuration options, see ShaderConfiguration.
	 */
	ShaderConfiguration configuration{
		.depthBufferMode = DepthBufferMode::IGNORE,
		.faceCullingMode = FaceCullingMode::IGNORE,
		.alphaMode = AlphaMode::USE_ALPHA_BLENDING,
	};
};

/**
 * ShaderProgram specialized for rendering TexturedQuad instances in 2D.
 */
class Shader2D {
public:
	/**
	 * Pointer to a statically allocated string containing the GLSL source code
	 * for a plain vertex shader.
	 */
	static const char* const vertexShaderSourceCodeInstancedTexturedQuad;

	/**
	 * Pointer to a statically allocated string containing the GLSL source code
	 * for a plain fragment shader.
	 */
	static const char* const fragmentShaderSourceCodeTexturedQuadPlain;

	/**
	 * Pointer to a statically allocated string containing the GLSL source code
	 * for a fragment shader that interprets the red channel as alpha with a
	 * white base color.
	 */
	static const char* const fragmentShaderSourceCodeTexturedQuadAlpha;

	/**
	 * Pointer to the statically allocated storage for the built-in plain
	 * shader.
	 *
	 * \warning This pointer must not be dereferenced in application code. It is
	 *          not guaranteed that the underlying shader will be present at all
	 *          times.
	 */
	static Shader2D* const plainShader;

	/**
	 * Pointer to the statically allocated storage for the built-in alpha
	 * shader.
	 *
	 * \warning This pointer must not be dereferenced in application code. It is
	 *          not guaranteed that the underlying shader will be present at all
	 *          times.
	 */
	static Shader2D* const alphaShader;

	/**
	 * Shader configuration that was supplied in the constructor.
	 */
	Shader2DOptions options;

	/**
	 * Shader program.
	 */
	ShaderProgram program;

	/**
	 * Identifier for the uniform shader variable for the projection-view
	 * matrix.
	 */
	ShaderUniform projectionViewMatrix{program, "projectionViewMatrix"};

	/**
	 * Identifier for the uniform shader variable for the main texture unit.
	 */
	ShaderUniform textureUnit{program, "textureUnit"};

	/**
	 * Compile and link a 2D shader program.
	 *
	 * \param programOptions base options for the ShaderProgram, see
	 *        ShaderProgramOptions.
	 * \param options additional options for the shader, see Shader2DOptions.
	 *
	 * \throws graphics::Error on failure to create a shader object, create the
	 *         shader program object, compile the shader code or link the shader
	 *         program.
	 * \throws std::bad_alloc on allocation failure.
	 */
	Shader2D(const ShaderProgramOptions& programOptions, const Shader2DOptions& options = {})
		: options(options)
		, program(programOptions) {}

private:
	friend Renderer;

	static void createSharedShaders();
	static void destroySharedShaders() noexcept;
};

} // namespace graphics
} // namespace donut

#endif
