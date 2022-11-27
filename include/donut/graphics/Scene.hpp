#ifndef DONUT_GRAPHICS_SCENE_HPP
#define DONUT_GRAPHICS_SCENE_HPP

#include <donut/graphics/Mesh.hpp>
#include <donut/graphics/Texture.hpp>

#include <cstddef>
#include <cstdint>
#include <glm/fwd.hpp>
#include <vector>

namespace donut {
namespace graphics {

struct Scene {
	struct Object final {
		struct Vertex {
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec3 tangent;
			glm::vec3 bitangent;
			glm::vec2 textureCoordinates;
		};

		using Index = std::uint32_t;

		struct Instance {
			glm::mat4 transformation;
			glm::mat3 normalMatrix;
			glm::vec4 tintColor;
		};

		struct Material {
			Texture diffuseMap;
			Texture specularMap;
			Texture normalMap;
			float specularExponent;
		};

		static constexpr MeshBufferUsage VERTICES_USAGE = MeshBufferUsage::STATIC_DRAW;
		static constexpr MeshBufferUsage INDICES_USAGE = MeshBufferUsage::STATIC_DRAW;
		static constexpr MeshBufferUsage INSTANCES_USAGE = MeshBufferUsage::STREAM_DRAW;

		static constexpr MeshPrimitiveType PRIMITIVE_TYPE = MeshPrimitiveType::TRIANGLES;
		static constexpr MeshIndexType INDEX_TYPE = MeshIndexType::U32;

		static constexpr std::int32_t TEXTURE_UNIT_DIFFUSE = 0;
		static constexpr std::int32_t TEXTURE_UNIT_SPECULAR = 1;
		static constexpr std::int32_t TEXTURE_UNIT_NORMAL = 2;

		Mesh<Vertex, Index, Instance> mesh;
		Material material;
		std::size_t indexCount;
	};

	explicit Scene(const char* filepath);

	std::vector<Object> objects;
};

} // namespace graphics
} // namespace donut

#endif
