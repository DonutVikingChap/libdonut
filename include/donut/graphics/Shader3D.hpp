#ifndef DONUT_GRAPHICS_SHADER_3D_HPP
#define DONUT_GRAPHICS_SHADER_3D_HPP

#include <donut/graphics/ShaderProgram.hpp>
#include <donut/graphics/ShaderUniform.hpp>

namespace donut {
namespace graphics {

class Renderer; // Forward declaration, to avoid a circular include of Renderer.hpp.

/**
 * Configuration options for a Shader3D.
 */
struct Shader3DOptions {
	/**
	 * Order of this shader relative to other shaders in the same RenderPass.
	 *
	 * Lower indices are ordered before higher indices. The index of the
	 * built-in shader is 0. Indices may have any value, including negative, and
	 * are not required to be consecutive.
	 */
	int orderIndex = 1;

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
	 */
	bool useDepthTest = true;

	/**
	 * Don't render primitives that are facing away from the viewer.
	 *
	 * The facing is determined by the winding order of the vertices on each
	 * rendered primitive. A counter-clockwise winding represents a front-facing
	 * primitive that is facing towards the viewer and should be rendered, while
	 * a clockwise winding represents a back-facing primitive that is facing
	 * away from the viewer and should not be rendered.
	 */
	bool useBackfaceCulling = true;

	/**
	 * Blend the old and new pixel colors depending on the alpha value of the
	 * new pixel according to the standard "over" compositing operator for
	 * transparency.
	 */
	bool useAlphaBlending = false;
};

/**
 * ShaderProgram specialized for rendering Model instances in 3D.
 */
class Shader3D {
public:
	/**
	 * Pointer to a statically allocated string containing the GLSL source code
	 * for a plain vertex shader.
	 */
	static const char* const vertexShaderSourceCodeInstancedModel;

	/**
	 * Pointer to a statically allocated string containing the GLSL source code
	 * for a fragment shader that uses the Blinn-Phong shading model with a
	 * single basic point light at a fixed position.
	 *
	 * \note This basic shader should mainly be used for testing or debugging
	 *       since the light parameters are completely arbitrary. Proper 3D
	 *       applications should typically use a custom fragment shader that
	 *       receives the light positions and colors as uniform data instead.
	 */
	static const char* const fragmentShaderSourceCodeModelBlinnPhong;

	/**
	 * Pointer to the statically allocated storage for the built-in blinn-phong
	 * shader.
	 *
	 * \warning This pointer must not be dereferenced in application code. It is
	 *          not guaranteed that the underlying shader will be present at all
	 *          times.
	 */
	static Shader3D* const blinnPhongShader;

	/**
	 * Shader configuration that was supplied in the constructor.
	 */
	Shader3DOptions options;

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
	 * Identifier for the uniform shader variable for the texture unit of the
	 * active material's diffuse map.
	 */
	ShaderUniform diffuseMap{program, "diffuseMap"};

	/**
	 * Identifier for the uniform shader variable for the texture unit of the
	 * active material's specular map.
	 */
	ShaderUniform specularMap{program, "specularMap"};

	/**
	 * Identifier for the uniform shader variable for the texture unit of the
	 * active material's normal map.
	 */
	ShaderUniform normalMap{program, "normalMap"};

	/**
	 * Identifier for the uniform shader variable for the active material's
	 * specular exponent.
	 */
	ShaderUniform specularExponent{program, "specularExponent"};

	/**
	 * Compile and link a 3D shader program.
	 *
	 * \param programOptions base options for the ShaderProgram, see
	 *        ShaderProgramOptions.
	 * \param options additional options for the shader, see Shader3DOptions.
	 *
	 * \throws graphics::Error on failure to create a shader object, create the
	 *         shader program object, compile the shader code or link the shader
	 *         program.
	 * \throws std::bad_alloc on allocation failure.
	 */
	Shader3D(const ShaderProgramOptions& programOptions, const Shader3DOptions& options = {})
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
