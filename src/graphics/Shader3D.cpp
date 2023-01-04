#include <donut/graphics/Shader3D.hpp>

namespace donut {
namespace graphics {

const char* const Shader3D::vertexShaderSourceCodeInstancedModel = R"GLSL(
    layout(location = 0) in vec3 inPosition;
    layout(location = 1) in vec3 inNormal;
    layout(location = 2) in vec3 inTangent;
    layout(location = 3) in vec3 inBitangent;
    layout(location = 4) in vec2 inTextureCoordinates;
    layout(location = 5) in mat4 instanceTransformation;
    layout(location = 9) in mat3 instanceNormalMatrix;
    layout(location = 12) in vec4 instanceTintColor;

    out vec3 ioFragmentPosition;
    out vec3 ioNormal;
    out vec3 ioTangent;
    out vec3 ioBitangent;
    out vec2 ioTextureCoordinates;
    out vec4 ioTintColor;

    uniform mat4 projectionViewMatrix;

    void main() {
        ioFragmentPosition = vec3(instanceTransformation * vec4(inPosition, 1.0));
        ioNormal = instanceNormalMatrix * inNormal;
        ioTangent = instanceNormalMatrix * inTangent;
        ioBitangent = instanceNormalMatrix * inBitangent;
        ioTextureCoordinates = inTextureCoordinates;
        ioTintColor = instanceTintColor;
        gl_Position = projectionViewMatrix * vec4(ioFragmentPosition, 1.0);
    }
)GLSL";

const char* const Shader3D::fragmentShaderSourceCodeModelBlinnPhong = R"GLSL(
    struct PointLight {
        vec3 position;
        vec3 ambient;
        vec3 diffuse;
        vec3 specular;
        float constantFalloff;
        float linearFalloff;
        float quadraticFalloff;
    };

    const uint POINT_LIGHT_COUNT = uint(1);
    const PointLight POINT_LIGHTS[POINT_LIGHT_COUNT] = PointLight[POINT_LIGHT_COUNT](
        PointLight(
            vec3(0.4, 1.6, 1.8), // position
            vec3(0.2, 0.2, 0.2), // ambient
            vec3(0.8, 0.8, 0.8), // diffuse
            vec3(0.8, 0.8, 0.8), // specular
            1.0, // constantFalloff
            0.04, // linearFalloff
            0.012 // quadraticFalloff
        )
    );

    const vec3 VIEW_POSITION = vec3(0.0, 0.0, 0.0);

    in vec3 ioFragmentPosition;
    in vec3 ioNormal;
    in vec3 ioTangent;
    in vec3 ioBitangent;
    in vec2 ioTextureCoordinates;
    in vec4 ioTintColor;

    out vec4 outFragmentColor;

    uniform sampler2D diffuseMap;
    uniform sampler2D specularMap;
    uniform sampler2D normalMap;
    uniform float specularExponent;

    float halfLambert(float cosine) {
        float factor = 0.5 + 0.5 * cosine;
        return factor * factor;
    }

    float blinnPhong(vec3 normal, vec3 lightDirection, vec3 viewDirection) {
        vec3 halfwayDirection = normalize(lightDirection + viewDirection);
        return pow(max(dot(normal, halfwayDirection), 0.0), specularExponent);
    }

    vec3 calculatePointLight(PointLight light, vec3 normal, vec3 viewDirection, vec3 ambientColor, vec3 diffuseColor, vec3 specularColor) {
        vec3 lightDifference = light.position - ioFragmentPosition;
        float lightDistanceSquared = dot(lightDifference, lightDifference);
        float lightDistance = sqrt(lightDistanceSquared);
        vec3 lightDirection = lightDifference * (1.0 / lightDistance);
        float cosine = dot(normal, lightDirection);
        float diffuseFactor = halfLambert(cosine);
        float specularFactor = blinnPhong(normal, lightDirection, viewDirection);
        float attenuation = 1.0 / (light.constantFalloff + light.linearFalloff * lightDistance + light.quadraticFalloff * lightDistanceSquared);
        vec3 ambientTerm = light.ambient * ambientColor;
        vec3 diffuseTerm = light.diffuse * diffuseFactor * diffuseColor;
        vec3 specularTerm = light.specular * specularFactor * specularColor;
        const float visibility = 1.0;
        return attenuation * (ambientTerm + (diffuseTerm + specularTerm) * visibility);
    }

    void main() {
        vec4 diffuseColor = ioTintColor * texture(diffuseMap, ioTextureCoordinates);
        vec3 specularColor = texture(specularMap, ioTextureCoordinates).rgb;
        
        mat3 TBN = mat3(normalize(ioTangent), normalize(ioBitangent), normalize(ioNormal));
        vec3 surfaceNormal = texture(normalMap, ioTextureCoordinates).xyz * 2.0 - vec3(1.0);
        vec3 normal = normalize(TBN * surfaceNormal);

        vec3 viewDirection = normalize(VIEW_POSITION - ioFragmentPosition);

        vec3 result = vec3(0.0, 0.0, 0.0);
        for (uint i = uint(0); i < POINT_LIGHT_COUNT; ++i) {
            result += calculatePointLight(POINT_LIGHTS[i], normal, viewDirection, diffuseColor.rgb, diffuseColor.rgb, specularColor);
        }
        outFragmentColor = vec4(result, diffuseColor.a);
    }
)GLSL";

} // namespace graphics
} // namespace donut
