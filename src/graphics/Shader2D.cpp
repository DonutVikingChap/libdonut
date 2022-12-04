#include <donut/graphics/Shader2D.hpp>

#include <string_view> // std::string_view

namespace donut {
namespace graphics {

const std::string_view Shader2D::vertexShaderSourceCodeInstancedTexturedQuad = R"GLSL(
    layout(location = 0) in vec2 inCoordinates;
    layout(location = 1) in mat4 instanceTransformation;
    layout(location = 5) in vec2 instanceTextureOffset;
    layout(location = 6) in vec2 instanceTextureScale;
    layout(location = 7) in vec4 instanceTintColor;

    out vec2 ioTextureCoordinates;
    out vec4 ioTintColor;

    uniform mat4 projectionViewMatrix;

    void main() {
        ioTextureCoordinates = instanceTextureOffset + inCoordinates * instanceTextureScale;
        ioTintColor = instanceTintColor;
        gl_Position = projectionViewMatrix * instanceTransformation * vec4(inCoordinates, 0.0, 1.0);
    }
)GLSL";

const std::string_view Shader2D::fragmentShaderSourceCodeTexturedQuadPlain = R"GLSL(
    in vec2 ioTextureCoordinates;
    in vec4 ioTintColor;

    out vec4 outFragmentColor;

    uniform sampler2D textureUnit;

    void main() {
        outFragmentColor = ioTintColor * texture(textureUnit, ioTextureCoordinates);
    }
)GLSL";

const std::string_view Shader2D::fragmentShaderSourceCodeTexturedQuadAlpha = R"GLSL(
    in vec2 ioTextureCoordinates;
    in vec4 ioTintColor;

    out vec4 outFragmentColor;

    uniform sampler2D textureUnit;

    void main() {
        outFragmentColor = vec4(ioTintColor.rgb, ioTintColor.a * texture(textureUnit, ioTextureCoordinates).r);
    }
)GLSL";

} // namespace graphics
} // namespace donut
