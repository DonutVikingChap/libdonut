#ifndef DONUT_GRAPHICS_SHADER_3D_HPP
#define DONUT_GRAPHICS_SHADER_3D_HPP

#include <donut/graphics/ShaderConfiguration.hpp>
#include <donut/graphics/ShaderParameter.hpp>
#include <donut/graphics/ShaderProgram.hpp>

namespace donut::graphics {

class Renderer; // Forward declaration, to avoid a circular include of Renderer.hpp.

/**
 * Configuration options for a Shader3D.
 */
struct Shader3DOptions {
	/**
	 * Shader configuration options, see ShaderConfiguration.
	 */
	ShaderConfiguration configuration{};
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
	static const char* const VERTEX_SHADER_SOURCE_CODE_INSTANCED_MODEL;

	/**
	 * Pointer to a statically allocated string containing the GLSL source code
	 * for a fragment shader that uses a fullbright shading model with no
	 * lights.
	 *
	 * \note This shader uses only the diffuse texture and renders it at 100%
	 *       brightness without taking details such as specular highlights,
	 *       normal mapping or emissive color into account.
	 */
	static const char* const FRAGMENT_SHADER_SOURCE_CODE_UNLIT;

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
	static const char* const FRAGMENT_SHADER_SOURCE_CODE_BLINN_PHONG;

	/**
	 * Pointer to the statically allocated storage for the built-in unlit
	 * shader.
	 *
	 * \warning This pointer must not be dereferenced in application code. It is
	 *          not guaranteed that the underlying shader will be present at all
	 *          times.
	 */
	static Shader3D* const UNLIT;

	/**
	 * Pointer to the statically allocated storage for the built-in blinn-phong
	 * shader.
	 *
	 * \warning This pointer must not be dereferenced in application code. It is
	 *          not guaranteed that the underlying shader will be present at all
	 *          times.
	 */
	static Shader3D* const BLINN_PHONG;

	/**
	 * Shader configuration that was supplied in the constructor.
	 */
	Shader3DOptions options;

	/**
	 * Shader program.
	 */
	ShaderProgram program;

	/**
	 * Identifier for the uniform shader variable for the projection matrix.
	 */
	ShaderParameter projectionMatrix{program, "projectionMatrix"};

	/**
	 * Identifier for the uniform shader variable for the view matrix.
	 */
	ShaderParameter viewMatrix{program, "viewMatrix"};

	/**
	 * Identifier for the uniform shader variable for the combined
	 * view-projection matrix.
	 */
	ShaderParameter viewProjectionMatrix{program, "viewProjectionMatrix"};

	/**
	 * Identifier for the uniform shader variable for the texture unit of the
	 * active material's diffuse map.
	 */
	ShaderParameter diffuseMap{program, "diffuseMap"};

	/**
	 * Identifier for the uniform shader variable for the texture unit of the
	 * active material's specular map.
	 */
	ShaderParameter specularMap{program, "specularMap"};

	/**
	 * Identifier for the uniform shader variable for the texture unit of the
	 * active material's normal map.
	 */
	ShaderParameter normalMap{program, "normalMap"};

	/**
	 * Identifier for the uniform shader variable for the texture unit of the
	 * active material's emissive map.
	 */
	ShaderParameter emissiveMap{program, "emissiveMap"};

	/**
	 * Identifier for the uniform shader variable for the active material's
	 * diffuse color.
	 */
	ShaderParameter diffuseColor{program, "diffuseColor"};

	/**
	 * Identifier for the uniform shader variable for the active material's
	 * specular color.
	 */
	ShaderParameter specularColor{program, "specularColor"};

	/**
	 * Identifier for the uniform shader variable for the active material's
	 * normal scale.
	 */
	ShaderParameter normalScale{program, "normalScale"};

	/**
	 * Identifier for the uniform shader variable for the active material's
	 * emissive color.
	 */
	ShaderParameter emissiveColor{program, "emissiveColor"};

	/**
	 * Identifier for the uniform shader variable for the active material's
	 * specular exponent.
	 */
	ShaderParameter specularExponent{program, "specularExponent"};

	/**
	 * Identifier for the uniform shader variable for the active material's
	 * specular exponent.
	 */
	ShaderParameter dissolveFactor{program, "dissolveFactor"};

	/**
	 * Identifier for the uniform shader variable for the active material's
	 * occlusion factor.
	 */
	ShaderParameter occlusionFactor{program, "occlusionFactor"};

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

} // namespace donut::graphics

#endif
