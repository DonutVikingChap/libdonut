#ifndef DONUT_GRAPHICS_MODEL_HPP
#define DONUT_GRAPHICS_MODEL_HPP

#include <donut/graphics/Mesh.hpp>
#include <donut/graphics/Texture.hpp>

#include <cstddef>     // std::size_t
#include <cstdint>     // std::int32_t, std::uint32_t
#include <glm/glm.hpp> // glm::...
#include <vector>      // std::vector

namespace donut {
namespace graphics {

/**
 * Container of a set of 3D triangle meshes stored on the GPU, combined with
 * associated materials.
 */
struct Model {
	/**
	 * A single 3D mesh with an associated material.
	 *
	 * The vertices and indices of the mesh are stored on the GPU, together with
	 * an associated instance buffer set up for instanced rendering.
	 */
	struct Object final {
		/**
		 * Data layout for the attributes of a single vertex of the mesh.
		 *
		 * \note Meets the requirements of the donut::graphics::vertex concept.
		 */
		struct Vertex {
			glm::vec3 position;           ///< Position relative to the model origin.
			glm::vec3 normal;             ///< Unit vector pointing away from the vertex surface.
			glm::vec3 tangent;            ///< Unit vector pointing in some direction along the vertex surface.
			glm::vec3 bitangent;          ///< Unit vector that is the cross product of the normal and the tangent.
			glm::vec2 textureCoordinates; ///< Texture UV coordinates that map to this vertex.
		};

		/**
		 * Data type used in the index buffer of the mesh.
		 *
		 * \note Meets the requirements of the donut::graphics::index concept.
		 */
		using Index = std::uint32_t;

		/**
		 * Data layout for the attributes of a single instance of the mesh.
		 *
		 * \note Meets the requirements of the donut::graphics::instance
		 *       concept.
		 */
		struct Instance {
			glm::mat4 transformation; ///< Model transformation matrix.
			glm::mat3 normalMatrix;   ///< Transposed 3x3 basis of the model transformation matrix.
			glm::vec4 tintColor;      ///< Tint color to use when rendering.
		};

		/**
		 * Material attributes of the mesh.
		 */
		struct Material {
			Texture diffuseMap;     ///< Texture used for the base color.
			Texture specularMap;    ///< Texture used for specular highlights.
			Texture normalMap;      ///< Texture used for normal mapping.
			float specularExponent; ///< Specular exponent to use for specular highlights.
		};

		/** Hint regarding the intended memory access pattern of the vertex buffer. */
		static constexpr MeshBufferUsage VERTICES_USAGE = MeshBufferUsage::STATIC_DRAW;

		/** Hint regarding the intended memory access pattern of the index buffer. */
		static constexpr MeshBufferUsage INDICES_USAGE = MeshBufferUsage::STATIC_DRAW;

		/** Hint regarding the intended memory access pattern of the instance buffer. */
		static constexpr MeshBufferUsage INSTANCES_USAGE = MeshBufferUsage::STREAM_DRAW;

		/** The type of 3D primitives represented by the mesh vertices. */
		static constexpr MeshPrimitiveType PRIMITIVE_TYPE = MeshPrimitiveType::TRIANGLES;

		/** Index type of the mesh indices. */
		static constexpr MeshIndexType INDEX_TYPE = MeshIndexType::U32;

		static constexpr std::int32_t TEXTURE_UNIT_DIFFUSE = 0;  ///< Texture unit index to use for the Material::diffuseMap.
		static constexpr std::int32_t TEXTURE_UNIT_SPECULAR = 1; ///< Texture unit index to use for the Material::specularMap.
		static constexpr std::int32_t TEXTURE_UNIT_NORMAL = 2;   ///< Texture unit index to use for the Material::normalMap.

		/**
		 * Mesh data stored on the GPU.
		 */
		Mesh<Vertex, Index, Instance> mesh;

		/**
		 * Material attributes.
		 */
		Material material;

		/**
		 * Number of indices stored in the index buffer of the mesh.
		 */
		std::size_t indexCount;
	};

	/**
	 * Load a model from a virtual file.
	 *
	 * The supported file formats are:
	 * - Wavefront OBJ (.obj)
	 *
	 * \param filepath virtual filepath of the model file to load, see File.
	 *
	 * \throws File::Error on failure to open the file.
	 * \throws graphics::Error on failure to load a model from the file.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note The file format is determined entirely from the file contents; the
	 *       filename extension is not taken into account.
	 * \note Any material libraries and texture image files required by the
	 *       model are also loaded as needed. See the documentation of ImageLDR
	 *       and ImageHDR for a description of the supported image file formats.
	 */
	explicit Model(const char* filepath);

	/**
	 * List of objects defined by the loaded model.
	 */
	std::vector<Object> objects;
};

} // namespace graphics
} // namespace donut

#endif
