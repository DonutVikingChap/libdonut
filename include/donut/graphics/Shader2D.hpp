#ifndef DONUT_GRAPHICS_SHADER_2D_HPP
#define DONUT_GRAPHICS_SHADER_2D_HPP

#include <donut/graphics/Shader.hpp>

namespace donut {
namespace graphics {

class Renderer; // Forward declaration, to avoid a circular include of Renderer.hpp.

/**
 * Configuration options for a Shader2D.
 */
struct Shader2DOptions {
	/**
	 * Write any new depth values to the depth buffer while ignoring the old
	 * values.
	 */
	bool overwriteDepthBuffer = false;

	/**
	 * Perform a depth test against the depth buffer before rendering a fragment
	 * in order to avoid 3D objects and faces being incorrectly rendered on top
	 * of each other. If the new depth value is closer, it overrides the old
	 * value in the depth buffer.
	 *
	 * \note This should typically be disabled for 2D shaders since all vertices
	 *       on the z=0 plane share the same depth value.
	 */
	bool useDepthTest = false;

	/**
	 * Don't render primitives that are facing away from the viewer.
	 *
	 * The facing is determined by the winding order of the vertices on each
	 * rendered primitive. A counter-clockwise winding represents a front-facing
	 * primitive that is facing towards the viewer and should be rendered, while
	 * a clockwise winding represents a back-facing primitive that is facing
	 * away from the viewer and should not be rendered.
	 */
	bool useBackfaceCulling = false;

	/**
	 * Blend the old and new pixel colors depending on the alpha value of the
	 * new pixel according to the standard "over" compositing operator for
	 * transparency.
	 */
	bool useAlphaBlending = true;
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
