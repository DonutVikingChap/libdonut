#ifndef DONUT_GRAPHICS_TEXTURED_QUAD_HPP
#define DONUT_GRAPHICS_TEXTURED_QUAD_HPP

#include <donut/graphics/Mesh.hpp>
#include <donut/math.hpp>

#include <array>   // std::array
#include <cstdint> // std::int32_t

namespace donut::graphics {

/**
 * Square 2D mesh for textured rendering.
 *
 * The vertices of the mesh are stored on the GPU, together with an associated
 * instance buffer set up for instanced rendering with a single texture.
 */
struct TexturedQuad {
	/**
	 * Data layout for the attributes of a single vertex of the mesh.
	 *
	 * \note Meets the requirements of the donut::graphics::mesh_vertex concept.
	 */
	struct Vertex {
		vec2 coordinates; ///< Shared vertex position and texture coordinates.
	};

	/**
	 * Data layout for the attributes of a single instance of the mesh.
	 *
	 * \note Meets the requirements of the donut::graphics::mesh_instance
	 *       concept.
	 */
	struct Instance {
		mat3 transformation;        ///< Transformation to apply to the vertex positions.
		vec4 textureOffsetAndScale; ///< Texture offset (xy) and texture scale (zw) to apply to the texture coordinates before sampling the texture.
		vec4 tintColor;             ///< Tint color to use when rendering.
	};

	/** Hint regarding the intended memory access pattern of the vertex buffer. */
	static constexpr MeshBufferUsage VERTICES_USAGE = MeshBufferUsage::STATIC_DRAW;

	/** Hint regarding the intended memory access pattern of the instance buffer. */
	static constexpr MeshBufferUsage INSTANCES_USAGE = MeshBufferUsage::STREAM_DRAW;

	/** The type of 3D primitives represented by the mesh vertices. */
	static constexpr MeshPrimitiveType PRIMITIVE_TYPE = MeshPrimitiveType::TRIANGLE_STRIP;

	/** The constant vertex data stored in the mesh. */
	static constexpr std::array<Vertex, 4> VERTICES{{
		{.coordinates{0.0f, 0.0f}},
		{.coordinates{0.0f, 1.0f}},
		{.coordinates{1.0f, 0.0f}},
		{.coordinates{1.0f, 1.0f}},
	}};

	static constexpr std::int32_t TEXTURE_UNIT = 0;       ///< Main texture unit index to use in the shader.
	static constexpr std::int32_t TEXTURE_UNIT_COUNT = 1; ///< Total number of texture units required to render a textured quad.

	/**
	 * Mesh data stored on the GPU.
	 */
	Mesh<Vertex, NoIndex, Instance> mesh{VERTICES_USAGE, INSTANCES_USAGE, VERTICES, {}};
};

} // namespace donut::graphics

#endif
