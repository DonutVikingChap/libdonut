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
	flat = 0,
	lambert = 1,
	blinn_phong = 2,
	blinn_phong_raytrace = 3,
	blinn_phong_raytrace_glass = 4,
	blinn_phong_raytrace_fresnel = 5,
	blinn_phong_raytrace_refract = 6,
	blinn_phong_raytrace_refract_fresnel = 7,
	blinn_phong_reflect = 8,
	blinn_phong_reflect_glass = 9,
	shadow = 10,
	// NOTE: Keep "count" at the end of the enumeration!
	count,
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
	IlluminationModel illuminationModel = IlluminationModel::flat;
};

struct Library {
	[[nodiscard]] static Library parse(std::string_view mtlString);

	std::vector<Material> materials;
};

} // namespace mtl
} // namespace obj
} // namespace donut

#endif
