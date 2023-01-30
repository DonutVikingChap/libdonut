#include <donut/graphics/Shader2D.hpp>

namespace donut {
namespace graphics {

const char* const Shader2D::vertexShaderSourceCodeInstancedTexturedQuad = R"GLSL(
    layout(location = 0) in vec2 vertexCoordinates;
    layout(location = 1) in mat4 instanceTransformation;
    layout(location = 5) in vec2 instanceTextureOffset;
    layout(location = 6) in vec2 instanceTextureScale;
    layout(location = 7) in vec4 instanceTintColor;

    out vec2 fragmentTextureCoordinates;
    out vec4 fragmentTintColor;

    uniform mat4 projectionViewMatrix;

    void main() {
        fragmentTextureCoordinates = instanceTextureOffset + vertexCoordinates * instanceTextureScale;
        fragmentTintColor = instanceTintColor;
        gl_Position = projectionViewMatrix * instanceTransformation * vec4(vertexCoordinates, 0.0, 1.0);
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

} // namespace graphics
} // namespace donut
