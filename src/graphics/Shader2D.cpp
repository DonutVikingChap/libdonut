#include <donut/graphics/Shader2D.hpp>

#include <array>   // std::array
#include <cstddef> // std::size_t, std::byte
#include <memory>  // std::construct_at, std::destroy_at

namespace donut {
namespace graphics {

namespace {

std::size_t sharedShaderReferenceCount = 0;
alignas(Shader2D) std::array<std::byte, sizeof(Shader2D)> sharedPlainShaderStorage;
alignas(Shader2D) std::array<std::byte, sizeof(Shader2D)> sharedAlphaShaderStorage;

} // namespace

const char* const Shader2D::vertexShaderSourceCodeInstancedTexturedQuad = R"GLSL(
    layout(location = 0) in vec2 vertexCoordinates;
    layout(location = 1) in mat4 instanceTransformation;
    layout(location = 5) in vec2 instanceTextureOffset;
    layout(location = 6) in vec2 instanceTextureScale;
    layout(location = 7) in vec4 instanceTintColor;

    out vec2 fragmentTextureCoordinates;
    out vec4 fragmentTintColor;

    uniform mat4 projectionMatrix;
    uniform mat4 viewMatrix;
    uniform mat4 viewProjectionMatrix;

    void main() {
        fragmentTextureCoordinates = instanceTextureOffset + vertexCoordinates * instanceTextureScale;
        fragmentTintColor = instanceTintColor;
        gl_Position = viewProjectionMatrix * instanceTransformation * vec4(vertexCoordinates, 0.0, 1.0);
    }
)GLSL";

const char* const Shader2D::fragmentShaderSourceCodeTexturedQuadPlain = R"GLSL(
    in vec2 fragmentTextureCoordinates;
    in vec4 fragmentTintColor;

    out vec4 outputColor;

    uniform sampler2D textureUnit;

    void main() {
        outputColor = fragmentTintColor * texture(textureUnit, fragmentTextureCoordinates);
    }
)GLSL";

const char* const Shader2D::fragmentShaderSourceCodeTexturedQuadAlpha = R"GLSL(
    in vec2 fragmentTextureCoordinates;
    in vec4 fragmentTintColor;

    out vec4 outputColor;

    uniform sampler2D textureUnit;

    void main() {
        outputColor = vec4(fragmentTintColor.rgb, fragmentTintColor.a * texture(textureUnit, fragmentTextureCoordinates).r);
    }
)GLSL";

Shader2D* const Shader2D::plainShader = reinterpret_cast<Shader2D*>(sharedPlainShaderStorage.data());
Shader2D* const Shader2D::alphaShader = reinterpret_cast<Shader2D*>(sharedAlphaShaderStorage.data());

void Shader2D::createSharedShaders() {
	if (sharedShaderReferenceCount == 0) {
		std::construct_at(plainShader,
			ShaderProgramOptions{
				.vertexShaderSourceCode = vertexShaderSourceCodeInstancedTexturedQuad,
				.fragmentShaderSourceCode = fragmentShaderSourceCodeTexturedQuadPlain,
			},
			Shader2DOptions{});
		try {
			std::construct_at(alphaShader,
				ShaderProgramOptions{
					.vertexShaderSourceCode = vertexShaderSourceCodeInstancedTexturedQuad,
					.fragmentShaderSourceCode = fragmentShaderSourceCodeTexturedQuadAlpha,
				},
				Shader2DOptions{});
		} catch (...) {
			std::destroy_at(plainShader);
			throw;
		}
	}
	++sharedShaderReferenceCount;
}

void Shader2D::destroySharedShaders() noexcept {
	if (sharedShaderReferenceCount-- == 1) {
		std::destroy_at(alphaShader);
		std::destroy_at(plainShader);
	}
}

} // namespace graphics
} // namespace donut
