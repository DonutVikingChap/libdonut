#ifndef DONUT_OBJ_HPP
#define DONUT_OBJ_HPP

#include <donut/math.hpp>

#include <cstddef>     // std::size_t
#include <cstdint>     // std::uint8_t, std::uint32_t
#include <stdexcept>   // std::runtime_error
#include <string>      // std::string
#include <string_view> // std::string_view
#include <vector>      // std::vector

namespace donut::obj {

/**
 * Exception type for errors originating from the OBJ API.
 */
struct Error : std::runtime_error {
	/**
	 * Iterator into the source OBJ string where the error originated from.
	 */
	std::string_view::iterator position;

	/**
	 * Line number, starting at 1 for the first line, where the error occured.
	 */
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

/**
 * Single vertex of a polygonal Face element.
 */
struct FaceVertex {
	std::uint32_t vertexIndex = 0;            ///< Index of the vertex coordinates in the Scene that define the vertex position.
	std::uint32_t textureCoordinateIndex = 0; ///< Index of the texture coordinates in the Scene that define the texture coordinates of the vertex.
	std::uint32_t normalIndex = 0;            ///< Index of the normal vector in the Scene that define the vertex normal.
};

/**
 * Face element forming a single polygon of FaceVertex vertices.
 */
struct Face {
	std::vector<FaceVertex> vertices{}; ///< List of vertices that make up the polygon.
};

/**
 * Group containing polygonal Face elements within an Object.
 */
struct Group {
	std::string name{};         ///< Name of the group, or empty if no name was specified.
	std::vector<Face> faces{};  ///< List of faces belonging to this group.
	std::string materialName{}; ///< Name of the material of this group, which should be found in one of the associated material libraries.
};

/**
 * Object mesh containing Group elements within a Scene.
 */
struct Object {
	std::string name{};          ///< Name of the object, or empty if no name was specified.
	std::vector<Group> groups{}; ///< List of groups belonging to this object.
};

/**
 * Scene of Object elements defined by an OBJ file.
 */
struct Scene {
	/**
	 * Parse a scene from an OBJ string.
	 *
	 * \param objString read-only view over the OBJ string to parse.
	 *
	 * \return the parsed scene.
	 *
	 * \throws Error on failure to parse any element of the scene.
	 * \throws std::bad_alloc on allocation failure.
	 */
	[[nodiscard]] static Scene parse(std::string_view objString);

	std::vector<std::string> materialLibraryFilenames{}; ///< List of relative filepaths of the material libraries associated with this scene.
	std::vector<vec3> vertices{};                        ///< List of vertex positions referenced by the face vertices defined in this scene.
	std::vector<vec2> textureCoordinates{};              ///< List of texture coordinates referenced by the face vertices defined in this scene.
	std::vector<vec3> normals{};                         ///< List of normal vectors referenced by the face vertices defined in this scene.
	std::vector<Object> objects{};                       ///< List of objects belonging to this scene.
};

namespace mtl {

/**
 * Illumination model to use when rendering a specific Material.
 */
enum class IlluminationModel : std::uint8_t {
	FLAT = 0,                                 ///< Implementation-defined. \hideinitializer
	LAMBERT = 1,                              ///< Implementation-defined. \hideinitializer
	BLINN_PHONG = 2,                          ///< Implementation-defined. \hideinitializer
	BLINN_PHONG_RAYTRACE = 3,                 ///< Implementation-defined. \hideinitializer
	BLINN_PHONG_RAYTRACE_GLASS = 4,           ///< Implementation-defined. \hideinitializer
	BLINN_PHONG_RAYTRACE_FRESNEL = 5,         ///< Implementation-defined. \hideinitializer
	BLINN_PHONG_RAYTRACE_REFRACT = 6,         ///< Implementation-defined. \hideinitializer
	BLINN_PHONG_RAYTRACE_REFRACT_FRESNEL = 7, ///< Implementation-defined. \hideinitializer
	BLINN_PHONG_REFLECT = 8,                  ///< Implementation-defined. \hideinitializer
	BLINN_PHONG_REFLECT_GLASS = 9,            ///< Implementation-defined. \hideinitializer
	SHADOW = 10,                              ///< Implementation-defined. \hideinitializer
};

/**
 * Material properties of an Object.
 */
struct Material {
	std::string name{};                                            ///< Name of the material.
	std::string ambientMapName{};                                  ///< Relative filepath of the ambient map image, or empty for no ambient map.
	std::string diffuseMapName{};                                  ///< Relative filepath of the diffuse map image, or empty for no diffuse map.
	std::string specularMapName{};                                 ///< Relative filepath of the specular map image, or empty for no specular map.
	std::string emissiveMapName{};                                 ///< Relative filepath of the emissive map image, or empty for no emissive map.
	std::string specularExponentMapName{};                         ///< Relative filepath of the specular exponent map image, or empty for no specular exponent map.
	std::string dissolveFactorMapName{};                           ///< Relative filepath of the dissolve factor map image, or empty for no dissolve factor map.
	std::string bumpMapName{};                                     ///< Relative filepath of the bump/normal map image, or empty for no bump/normal map.
	vec3 ambientColor{1.0f, 1.0f, 1.0f};                           ///< Ambient color factor to multiply the sampled ambient map value by.
	vec3 diffuseColor{1.0f, 1.0f, 1.0f};                           ///< Diffuse color factor to multiply the sampled diffuse map value by.
	vec3 specularColor{1.0f, 1.0f, 1.0f};                          ///< Specular color factor to multiply the sampled specular map value by.
	vec3 emissiveColor{0.0f, 0.0f, 0.0f};                          ///< Emissive color factor to multiply the sampled emissive map value by.
	float specularExponent = 1.0f;                                 ///< Specular exponent factor to multiply the sampled specular exponent map value by.
	float dissolveFactor = 0.0f;                                   ///< Dissolve factor to multiply the sampled dissolve factor map value by.
	IlluminationModel illuminationModel = IlluminationModel::FLAT; ///< Illumination model to use for rendering this material.
};

/**
 * Material library that stores the material properties for objects defined in a
 * Scene.
 */
struct Library {
	/**
	 * Parse a material library from an MTL string.
	 *
	 * \param mtlString read-only view over the MTL string to parse.
	 *
	 * \return the parsed material library.
	 *
	 * \throws Error on failure to parse any element of the material library.
	 * \throws std::bad_alloc on allocation failure.
	 */
	[[nodiscard]] static Library parse(std::string_view mtlString);

	std::vector<Material> materials{}; ///< List of materials belonging to this library.
};

} // namespace mtl
} // namespace donut::obj

#endif
