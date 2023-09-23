#ifndef DONUT_GRAPHICS_MODEL_HPP
#define DONUT_GRAPHICS_MODEL_HPP

#include <donut/Filesystem.hpp>
#include <donut/graphics/Mesh.hpp>
#include <donut/graphics/Texture.hpp>
#include <donut/math.hpp>

#include <cstddef> // std::size_t
#include <vector>  // std::vector

namespace donut::graphics {

class Renderer; // Forward declaration, to avoid a circular include of Renderer.hpp.

/**
 * Container for a set of 3D triangle meshes stored on the GPU, combined with
 * associated materials.
 */
struct Model {
	/**
	 * A single 3D mesh with an associated material.
	 *
	 * The vertices and indices of the mesh are stored on the GPU, together with
	 * an associated instance buffer set up for instanced rendering.
	 */
	struct Object {
		/**
		 * Data layout for the attributes of a single vertex of the mesh.
		 *
		 * \note Meets the requirements of the donut::graphics::mesh_vertex
		 *       concept.
		 */
		struct Vertex {
			vec3 position;           ///< Position relative to the model origin.
			vec3 normal;             ///< Unit vector pointing away from the vertex surface.
			vec3 tangent;            ///< Unit vector pointing in some direction along the vertex surface.
			vec3 bitangent;          ///< Unit vector that is the cross product of the normal and the tangent.
			vec2 textureCoordinates; ///< Texture UV coordinates that map to this vertex.
		};

		/**
		 * Data type used in the index buffer of the mesh.
		 *
		 * \note Meets the requirements of the donut::graphics::mesh_index
		 *       concept.
		 */
		using Index = u32;

		/**
		 * Data layout for the attributes of a single instance of the mesh.
		 *
		 * \note Meets the requirements of the donut::graphics::mesh_instance
		 *       concept.
		 */
		struct Instance {
			mat4 transformation;        ///< Model transformation matrix.
			mat3 normalMatrix;          ///< Transposed 3x3 basis of the model transformation matrix.
			vec4 textureOffsetAndScale; ///< Texture offset (xy) and texture scale (zw) to apply to the texture coordinates before sampling the texture.
			vec4 tintColor;             ///< Tint color to use when rendering.
			vec3 specularFactor;        ///< Specular factor to use when rendering.
			vec3 emissiveFactor;        ///< Emissive factor to use when rendering.
		};

		/**
		 * Material attributes of the mesh.
		 */
		struct Material {
			Texture diffuseMap;     ///< Texture used for the base color.
			Texture specularMap;    ///< Texture used for specular highlights.
			Texture normalMap;      ///< Texture used for normal mapping.
			Texture emissiveMap;    ///< Texture used for emissive mapping.
			vec3 diffuseColor;      ///< Base color.
			vec3 specularColor;     ///< Specular color.
			vec3 normalScale;       ///< Normal map scale.
			vec3 emissiveColor;     ///< Emissive color.
			float specularExponent; ///< Specular exponent for specular highlights.
			float dissolveFactor;   ///< Dissolve factor for transparency.
			float occlusionFactor;  ///< Occlusion factor.
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
		static constexpr std::int32_t TEXTURE_UNIT_EMISSIVE = 3; ///< Texture unit index to use for the Material::emissiveMap.
		static constexpr std::int32_t TEXTURE_UNIT_COUNT = 4;    ///< Total number of texture units required to render an object.

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
	 * Pointer to the statically allocated storage for the built-in quad model.
	 *
	 * \warning This pointer must not be dereferenced in application code. It is
	 *          not guaranteed that the underlying model will be present at all
	 *          times.
	 */
	static const Model* const QUAD;

	/**
	 * Pointer to the statically allocated storage for the built-in cube model.
	 *
	 * \warning This pointer must not be dereferenced in application code. It is
	 *          not guaranteed that the underlying model will be present at all
	 *          times.
	 */
	static const Model* const CUBE;

	/**
	 * Construct a model from a list of meshes.
	 *
	 * \param objects meshes that define the model.
	 */
	explicit Model(std::vector<Object> objects) noexcept
		: objects(std::move(objects)) {}

	/**
	 * Load a model from a virtual file.
	 *
	 * The supported file formats are:
	 * - Wavefront OBJ (.obj)
	 *
	 * \param filesystem virtual filepath to load the files from.
	 * \param filepath virtual filepath of the model file to load.
	 *
	 * \throws File::Error on failure to open the file.
	 * \throws graphics::Error on failure to load a model from the file.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note The file format is determined entirely from the file contents; the
	 *       filename extension is not taken into account.
	 * \note Any material libraries and texture image files required by the
	 *       model are also loaded as needed. See the documentation of Image for
	 *       a description of the supported image file formats.
	 */
	explicit Model(const Filesystem& filesystem, const char* filepath);

	/**
	 * List of objects defined by the loaded model.
	 */
	std::vector<Object> objects;

private:
	friend Renderer;

	static void createSharedModels();
	static void destroySharedModels() noexcept;
};

} // namespace donut::graphics

#endif
