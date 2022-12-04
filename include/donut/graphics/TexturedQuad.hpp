#ifndef DONUT_GRAPHICS_TEXTURED_QUAD_HPP
#define DONUT_GRAPHICS_TEXTURED_QUAD_HPP

#include <donut/graphics/Mesh.hpp>

#include <array>       // std::array
#include <cstdint>     // std::int32_t
#include <glm/glm.hpp> // glm::...

namespace donut {
namespace graphics {

struct TexturedQuad {
	struct Vertex {
		glm::vec2 coordinates;
	};

	struct Instance {
		glm::mat4 transformation;
		glm::vec2 textureOffset;
		glm::vec2 textureScale;
		glm::vec4 tintColor;
	};

	static constexpr MeshBufferUsage VERTICES_USAGE = MeshBufferUsage::STATIC_DRAW;
	static constexpr MeshBufferUsage INSTANCES_USAGE = MeshBufferUsage::STREAM_DRAW;

	static constexpr MeshPrimitiveType PRIMITIVE_TYPE = MeshPrimitiveType::TRIANGLE_STRIP;
	static constexpr std::array<Vertex, 4> VERTICES{{
		{.coordinates{0.0f, 0.0f}},
		{.coordinates{0.0f, 1.0f}},
		{.coordinates{1.0f, 0.0f}},
		{.coordinates{1.0f, 1.0f}},
	}};

	static constexpr std::int32_t TEXTURE_UNIT = 0;

	Mesh<Vertex, NoIndex, Instance> mesh{VERTICES_USAGE, INSTANCES_USAGE, VERTICES, {}};
};

} // namespace graphics
} // namespace donut

#endif
