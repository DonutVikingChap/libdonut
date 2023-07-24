#include <donut/graphics/Shader3D.hpp>

#include <array>   // std::array
#include <cstddef> // std::size_t, std::byte
#include <memory>  // std::construct_at, std::destroy_at

namespace donut::graphics {

namespace {

std::size_t sharedShaderReferenceCount = 0;
alignas(Shader3D) std::array<std::byte, sizeof(Shader3D)> sharedBlinnPhongShaderStorage;

} // namespace

const char* const Shader3D::vertexShaderSourceCodeInstancedModel = R"GLSL(
    layout(location = 0) in vec3 vertexPosition;
    layout(location = 1) in vec3 vertexNormal;
    layout(location = 2) in vec3 vertexTangent;
    layout(location = 3) in vec3 vertexBitangent;
    layout(location = 4) in vec2 vertexTextureCoordinates;
    layout(location = 5) in mat4 instanceTransformation;
    layout(location = 9) in mat3 instanceNormalMatrix;
    layout(location = 12) in vec4 instanceTintColor;

    out vec3 fragmentPosition;
    out vec3 fragmentNormal;
    out vec3 fragmentTangent;
    out vec3 fragmentBitangent;
    out vec2 fragmentTextureCoordinates;
    out vec4 fragmentTintColor;

    uniform mat4 projectionMatrix;
    uniform mat4 viewMatrix;
    uniform mat4 viewProjectionMatrix;

    void main() {
        fragmentPosition = vec3(instanceTransformation * vec4(vertexPosition, 1.0));
        fragmentNormal = instanceNormalMatrix * vertexNormal;
        fragmentTangent = instanceNormalMatrix * vertexTangent;
        fragmentBitangent = instanceNormalMatrix * vertexBitangent;
        fragmentTextureCoordinates = vertexTextureCoordinates;
        fragmentTintColor = instanceTintColor;
        gl_Position = viewProjectionMatrix * vec4(fragmentPosition, 1.0);
    }
)GLSL";

const char* const Shader3D::fragmentShaderSourceCodeModelBlinnPhong = R"GLSL(
    #ifndef GAMMA
    #define GAMMA 2.2
    #endif

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
            vec3(0.005, 0.005, 0.005), // ambient
            vec3(0.8, 0.8, 0.8), // diffuse
            vec3(0.8, 0.8, 0.8), // specular
            1.0, // constantFalloff
            0.04, // linearFalloff
            0.012 // quadraticFalloff
        )
    );

    const vec3 VIEW_POSITION = vec3(0.0, 0.0, 0.0);

    in vec3 fragmentPosition;
    in vec3 fragmentNormal;
    in vec3 fragmentTangent;
    in vec3 fragmentBitangent;
    in vec2 fragmentTextureCoordinates;
    in vec4 fragmentTintColor;

    out vec4 outputColor;

    uniform sampler2D diffuseMap;
    uniform sampler2D specularMap;
    uniform sampler2D normalMap;
    uniform sampler2D emissiveMap;
    uniform vec3 diffuseColor;
    uniform vec3 specularColor;
    uniform vec3 normalScale;
    uniform vec3 emissiveColor;
    uniform float specularExponent;
    uniform float dissolveFactor;
    uniform float occlusionFactor;

    float halfLambert(float cosine) {
        float factor = 0.5 + 0.5 * cosine;
        return factor * factor;
    }

    float blinnPhong(vec3 normal, vec3 lightDirection, vec3 viewDirection) {
        vec3 halfwayDirection = normalize(lightDirection + viewDirection);
        return pow(max(dot(normal, halfwayDirection), 0.0), specularExponent);
    }

    vec3 calculatePointLight(PointLight light, vec3 normal, vec3 viewDirection, vec3 ambient, vec3 diffuse, vec3 specular) {
        vec3 lightDifference = light.position - fragmentPosition;
        float lightDistanceSquared = dot(lightDifference, lightDifference);
        float lightDistance = sqrt(lightDistanceSquared);
        vec3 lightDirection = lightDifference * (1.0 / lightDistance);
        float cosine = dot(normal, lightDirection);
        float diffuseFactor = halfLambert(cosine);
        float specularFactor = blinnPhong(normal, lightDirection, viewDirection);
        float attenuation = 1.0 / (light.constantFalloff + light.linearFalloff * lightDistance + light.quadraticFalloff * lightDistanceSquared);
        vec3 ambientTerm = light.ambient * ambient;
        vec3 diffuseTerm = light.diffuse * diffuseFactor * diffuse;
        vec3 specularTerm = light.specular * specularFactor * specular;
        const float visibility = 1.0;
        return attenuation * (ambientTerm * occlusionFactor + (diffuseTerm + specularTerm) * visibility);
    }

    void main() {
        vec4 sampledDiffuse = texture(diffuseMap, fragmentTextureCoordinates);
        vec4 diffuse = fragmentTintColor * vec4(diffuseColor, 1.0 - dissolveFactor) * vec4(pow(sampledDiffuse.rgb, vec3(GAMMA)), sampledDiffuse.a);
        vec3 specular = specularColor * texture(specularMap, fragmentTextureCoordinates).rgb;
        vec3 emissive = emissiveColor * texture(emissiveMap, fragmentTextureCoordinates).rgb;
        
        mat3 TBN = mat3(normalize(fragmentTangent), normalize(fragmentBitangent), normalize(fragmentNormal));
        vec3 surfaceNormal = normalScale * (texture(normalMap, fragmentTextureCoordinates).xyz * 2.0 - vec3(1.0));
        vec3 normal = normalize(TBN * surfaceNormal);

        vec3 viewDirection = normalize(VIEW_POSITION - fragmentPosition);

        vec3 color = emissive;
        for (uint i = uint(0); i < uint(POINT_LIGHT_COUNT); ++i) {
            color += calculatePointLight(POINT_LIGHTS[i], normal, viewDirection, vec3(1.0), diffuse.rgb, specular);
        }
        outputColor = vec4(pow(color, vec3(1.0 / GAMMA)), diffuse.a);
    }
)GLSL";

Shader3D* const Shader3D::blinnPhongShader = reinterpret_cast<Shader3D*>(sharedBlinnPhongShaderStorage.data());

void Shader3D::createSharedShaders() {
	if (sharedShaderReferenceCount == 0) {
		std::construct_at(blinnPhongShader,
			ShaderProgramOptions{
				.vertexShaderSourceCode = vertexShaderSourceCodeInstancedModel,
				.fragmentShaderSourceCode = fragmentShaderSourceCodeModelBlinnPhong,
			},
			Shader3DOptions{.orderIndex = 0});
	}
	++sharedShaderReferenceCount;
}

void Shader3D::destroySharedShaders() noexcept {
	if (sharedShaderReferenceCount-- == 1) {
		std::destroy_at(blinnPhongShader);
	}
}

} // namespace donut::graphics
