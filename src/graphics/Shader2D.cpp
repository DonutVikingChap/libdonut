#include <donut/graphics/Shader2D.hpp>

#include <array>   // std::array
#include <cstddef> // std::size_t, std::byte
#include <memory>  // std::construct_at, std::destroy_at

namespace donut::graphics {

namespace {

std::size_t sharedShaderReferenceCount = 0;
alignas(Shader2D) std::array<std::byte, sizeof(Shader2D)> sharedPlainShaderStorage;
alignas(Shader2D) std::array<std::byte, sizeof(Shader2D)> sharedAlphaShaderStorage;

} // namespace

const char* const Shader2D::VERTEX_SHADER_SOURCE_CODE_INSTANCED_TEXTURED_QUAD = R"GLSL(
    layout(location = 0) in vec2 vertexCoordinates;
    layout(location = 1) in mat3 instanceTransformation;
    layout(location = 4) in vec4 instanceTextureOffsetAndScale;
    layout(location = 5) in vec4 instanceTintColor;

    out vec2 fragmentTextureCoordinates;
    out vec4 fragmentTintColor;

    uniform mat4 projectionMatrix;
    uniform mat4 viewMatrix;
    uniform mat4 viewProjectionMatrix;

    void main() {
        fragmentTextureCoordinates = instanceTextureOffsetAndScale.xy + vertexCoordinates * instanceTextureOffsetAndScale.zw;
        fragmentTintColor = instanceTintColor;
        gl_Position = viewProjectionMatrix * vec4(instanceTransformation * vec3(vertexCoordinates, 1.0), 1.0);
    }
)GLSL";

const char* const Shader2D::FRAGMENT_SHADER_SOURCE_CODE_PLAIN = R"GLSL(
    in vec2 fragmentTextureCoordinates;
    in vec4 fragmentTintColor;

    out vec4 outputColor;

    uniform sampler2D textureUnit;

    void main() {
        outputColor = fragmentTintColor * texture(textureUnit, fragmentTextureCoordinates);
    }
)GLSL";

const char* const Shader2D::FRAGMENT_SHADER_SOURCE_CODE_ALPHA = R"GLSL(
    in vec2 fragmentTextureCoordinates;
    in vec4 fragmentTintColor;

    out vec4 outputColor;

    uniform sampler2D textureUnit;

    void main() {
        outputColor = vec4(fragmentTintColor.rgb, fragmentTintColor.a * texture(textureUnit, fragmentTextureCoordinates).r);
    }
)GLSL";

Shader2D* const Shader2D::PLAIN = reinterpret_cast<Shader2D*>(sharedPlainShaderStorage.data());
Shader2D* const Shader2D::ALPHA = reinterpret_cast<Shader2D*>(sharedAlphaShaderStorage.data());

void Shader2D::createSharedShaders() {
	if (sharedShaderReferenceCount == 0) {
		std::construct_at(PLAIN,
			ShaderProgramOptions{
				.vertexShaderSourceCode = VERTEX_SHADER_SOURCE_CODE_INSTANCED_TEXTURED_QUAD,
				.fragmentShaderSourceCode = FRAGMENT_SHADER_SOURCE_CODE_PLAIN,
			},
			Shader2DOptions{});
		try {
			std::construct_at(ALPHA,
				ShaderProgramOptions{
					.vertexShaderSourceCode = VERTEX_SHADER_SOURCE_CODE_INSTANCED_TEXTURED_QUAD,
					.fragmentShaderSourceCode = FRAGMENT_SHADER_SOURCE_CODE_ALPHA,
				},
				Shader2DOptions{});
		} catch (...) {
			std::destroy_at(PLAIN);
			throw;
		}
	}
	++sharedShaderReferenceCount;
}

void Shader2D::destroySharedShaders() noexcept {
	if (sharedShaderReferenceCount-- == 1) {
		std::destroy_at(ALPHA);
		std::destroy_at(PLAIN);
	}
}

} // namespace donut::graphics
