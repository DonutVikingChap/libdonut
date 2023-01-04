#ifndef DONUT_OBJ_HPP
#define DONUT_OBJ_HPP

#include <cstddef>     // std::size_t
#include <cstdint>     // std::uint32_t
#include <glm/glm.hpp> // glm::...
#include <stdexcept>   // std::runtime_error
#include <string>      // std::string
#include <string_view> // std::string_view
#include <vector>      // std::vector

namespace donut {
namespace obj {

struct Error : std::runtime_error {
	std::string_view::iterator position;
	std::size_t lineNumber;

	Error(const std::string& message, std::string_view::iterator position, std::size_t lineNumber)
		: std::runtime_error(message)
		, position(position)
		, lineNumber(lineNumber) {}

	Error(const char* message, std::string_view::iterator position, std::size_t lineNumber)
		: std::runtime_error(message)
		, position(position)
		, lineNumber(lineNumber) {}
};

struct FaceVertex {
	std::uint32_t vertexIndex = 0;
	std::uint32_t textureCoordinateIndex = 0;
	std::uint32_t normalIndex = 0;
};

struct Face {
	std::vector<FaceVertex> vertices{};
};

struct Group {
	std::string name{};
	std::vector<Face> faces{};
};

struct Object {
	std::string name{};
	std::string materialName{};
	std::vector<Group> groups{};
};

struct Scene {
	[[nodiscard]] static Scene parse(std::string_view objString);

	std::vector<std::string> materialLibraryFilenames{};
	std::vector<glm::vec3> vertices{};
	std::vector<glm::vec2> textureCoordinates{};
	std::vector<glm::vec3> normals{};
	std::vector<Object> objects{};
};

namespace mtl {

enum class IlluminationModel : std::uint8_t {
	FLAT = 0,
	LAMBERT = 1,
	BLINN_PHONG = 2,
	BLINN_PHONG_RAYTRACE = 3,
	BLINN_PHONG_RAYTRACE_GLASS = 4,
	BLINN_PHONG_RAYTRACE_FRESNEL = 5,
	BLINN_PHONG_RAYTRACE_REFRACT = 6,
	BLINN_PHONG_RAYTRACE_REFRACT_FRESNEL = 7,
	BLINN_PHONG_REFLECT = 8,
	BLINN_PHONG_REFLECT_GLASS = 9,
	SHADOW = 10,
	// NOTE: Keep "COUNT" at the end of the enumeration!
	COUNT,
};

struct Material {
	std::string name{};
	std::string ambientMapName{};
	std::string diffuseMapName{};
	std::string specularMapName{};
	std::string emissiveMapName{};
	std::string specularExponentMapName{};
	std::string dissolveFactorMapName{};
	std::string bumpMapName{};
	glm::vec3 ambientColor{};
	glm::vec3 diffuseColor{};
	glm::vec3 specularColor{};
	glm::vec3 emissiveColor{};
	float specularExponent = 0.0f;
	float dissolveFactor = 0.0f;
	IlluminationModel illuminationModel = IlluminationModel::FLAT;
};

struct Library {
	[[nodiscard]] static Library parse(std::string_view mtlString);

	std::vector<Material> materials;
};

} // namespace mtl
} // namespace obj
} // namespace donut

#endif
