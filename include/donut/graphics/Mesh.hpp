#ifndef DONUT_GRAPHICS_MESH_HPP
#define DONUT_GRAPHICS_MESH_HPP

#include <donut/graphics/Buffer.hpp>
#include <donut/graphics/Handle.hpp>
#include <donut/graphics/VertexArray.hpp>
#include <donut/reflection.hpp>

#include <cstddef>     // std::size_t, std::byte
#include <cstdint>     // std::int32_t, std::uint8_t, std::uint16_t, std::uint32_t, std::uintptr_t
#include <glm/glm.hpp> // glm::...
#include <memory>      // std::addressof
#include <span>        // std::span
#include <stdexcept>   // std::invalid_argument
#include <type_traits> // std::is_same_v, std::remove_cvref_t, std::false_type, std::true_type, std::bool_constant, std::is_aggregate_v, std::is_standard_layout_v, std::conditional_t
#include <utility>     // std::declval

namespace donut {
namespace graphics {

/**
 * Concept that checks if a type is a valid vertex attribute.
 *
 * \tparam T the type to check.
 */
template <typename T>
concept vertex_attribute =              //
	std::is_same_v<T, std::uint32_t> || //
	std::is_same_v<T, float> ||         //
	std::is_same_v<T, glm::vec2> ||     //
	std::is_same_v<T, glm::vec3> ||     //
	std::is_same_v<T, glm::vec4> ||     //
	std::is_same_v<T, glm::mat2> ||     //
	std::is_same_v<T, glm::mat3> ||     //
	std::is_same_v<T, glm::mat4>;

/**
 * Hint to the graphics driver implementation regarding the intended access
 * pattern of a particular GPU memory buffer in a Mesh, for optimization
 * purposes.
 *
 * See https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBufferData.xhtml
 * for details.
 */
enum class MeshBufferUsage : std::uint32_t {
	STATIC_COPY = 0x88E6,  ///< \hideinitializer
	STATIC_DRAW = 0x88E4,  ///< \hideinitializer
	STATIC_READ = 0x88E5,  ///< \hideinitializer
	DYNAMIC_COPY = 0x88EA, ///< \hideinitializer
	DYNAMIC_DRAW = 0x88E8, ///< \hideinitializer
	DYNAMIC_READ = 0x88E9, ///< \hideinitializer
	STREAM_COPY = 0x88E2,  ///< \hideinitializer
	STREAM_DRAW = 0x88E0,  ///< \hideinitializer
	STREAM_READ = 0x88E1,  ///< \hideinitializer
};

/**
 * Specification of which kind of graphical primitive is defined by an
 * associated sequence of vertices in a Mesh.
 */
enum class MeshPrimitiveType : std::uint32_t {
	POINTS = 0x0000,         ///< Individual points. \hideinitializer
	LINES = 0x0001,          ///< Each consecutive pair of points forms an individual line segment. \hideinitializer
	LINE_LOOP = 0x0002,      ///< Each point forms a line segment to the previous point, where the last point connects back to the first in a circular fashion. \hideinitializer
	LINE_STRIP = 0x0003,     ///< Each point, except the first, forms a line segment to the previous point. \hideinitializer
	TRIANGLES = 0x0004,      ///< Each consecutive triple of points forms an individual filled triangle. \hideinitializer
	TRIANGLE_STRIP = 0x0005, ///< Each point, except the first two, forms a filled triangle with the previous two points. \hideinitializer
};

/**
 * Specification of which type of indices is used in the index buffer of a
 * particular Mesh.
 */
enum class MeshIndexType : std::uint32_t {
	U8 = 0x1401,  ///< Unsigned 8-bit integer. \hideinitializer
	U16 = 0x1403, ///< Unsigned 16-bit integer. \hideinitializer
	U32 = 0x1405, ///< Unsigned 32-bit integer. \hideinitializer
};

namespace detail {

class MeshStatePreserver {
public:
	[[nodiscard]] MeshStatePreserver() noexcept;
	~MeshStatePreserver();

	MeshStatePreserver(const MeshStatePreserver&) = delete;
	MeshStatePreserver(MeshStatePreserver&&) = delete;
	MeshStatePreserver& operator=(const MeshStatePreserver&) = delete;
	MeshStatePreserver& operator=(MeshStatePreserver&&) = delete;

private:
	std::int32_t vertexArrayBinding = 0;
	std::int32_t arrayBufferBinding = 0;
};

void bindVertexArray(Handle handle);
void bindArrayBuffer(Handle handle);
void bindElementArrayBuffer(Handle handle);
void enableVertexAttribArray(std::uint32_t index);
void vertexAttribDivisor(std::uint32_t index, std::uint32_t divisor);
void vertexAttribPointerUint(std::uint32_t index, std::size_t count, std::size_t stride, std::uintptr_t offset);
void vertexAttribPointerFloat(std::uint32_t index, std::size_t count, std::size_t stride, std::uintptr_t offset);
void bufferArrayBufferData(std::size_t size, const void* data, MeshBufferUsage usage);
void bufferElementArrayBufferData(std::size_t size, const void* data, MeshBufferUsage usage);

template <bool IsInstance>
inline void enableVertexAttribute(std::uint32_t index) {
	enableVertexAttribArray(index);
	if constexpr (IsInstance) {
		vertexAttribDivisor(index, 1);
	}
}

template <bool IsInstance, typename T>
[[nodiscard]] inline std::uint32_t setupVertexAttribute(std::uint32_t index, std::size_t stride, std::uintptr_t offset) {
	if constexpr (std::is_same_v<T, std::uint32_t>) {
		enableVertexAttribute<IsInstance>(index);
		vertexAttribPointerUint(index++, 1, stride, offset);
	} else if constexpr (std::is_same_v<T, float>) {
		enableVertexAttribute<IsInstance>(index);
		vertexAttribPointerFloat(index++, 1, stride, offset);
	} else if constexpr (std::is_same_v<T, glm::vec2>) {
		enableVertexAttribute<IsInstance>(index);
		vertexAttribPointerFloat(index++, 2, stride, offset);
	} else if constexpr (std::is_same_v<T, glm::vec3>) {
		enableVertexAttribute<IsInstance>(index);
		vertexAttribPointerFloat(index++, 3, stride, offset);
	} else if constexpr (std::is_same_v<T, glm::vec4>) {
		enableVertexAttribute<IsInstance>(index);
		vertexAttribPointerFloat(index++, 4, stride, offset);
	} else if constexpr (std::is_same_v<T, glm::mat2>) {
		enableVertexAttribute<IsInstance>(index);
		vertexAttribPointerFloat(index++, 2, stride, offset);
		enableVertexAttribute<IsInstance>(index);
		vertexAttribPointerFloat(index++, 2, stride, offset + sizeof(float) * 2);
	} else if constexpr (std::is_same_v<T, glm::mat3>) {
		enableVertexAttribute<IsInstance>(index);
		vertexAttribPointerFloat(index++, 3, stride, offset);
		enableVertexAttribute<IsInstance>(index);
		vertexAttribPointerFloat(index++, 3, stride, offset + sizeof(float) * 3);
		enableVertexAttribute<IsInstance>(index);
		vertexAttribPointerFloat(index++, 3, stride, offset + sizeof(float) * 6);
	} else if constexpr (std::is_same_v<T, glm::mat4>) {
		enableVertexAttribute<IsInstance>(index);
		vertexAttribPointerFloat(index++, 4, stride, offset);
		enableVertexAttribute<IsInstance>(index);
		vertexAttribPointerFloat(index++, 4, stride, offset + sizeof(float) * 4);
		enableVertexAttribute<IsInstance>(index);
		vertexAttribPointerFloat(index++, 4, stride, offset + sizeof(float) * 8);
		enableVertexAttribute<IsInstance>(index);
		vertexAttribPointerFloat(index++, 4, stride, offset + sizeof(float) * 12);
	} else {
		throw std::invalid_argument{"Invalid vertex attribute type!"};
	}
	return index;
}

template <typename Tuple>
struct is_vertex_attributes : std::false_type {};

template <typename... Ts>
struct is_vertex_attributes<std::tuple<Ts...>> : std::bool_constant<(vertex_attribute<std::remove_cvref_t<Ts>> && ...)> {};

template <typename Tuple>
inline constexpr bool is_vertex_attributes_v = is_vertex_attributes<Tuple>::value;

} // namespace detail

/**
 * Concept that checks if a type is a valid vertex type.
 *
 * \tparam T the type to check.
 */
template <typename T>
concept vertex =                    //
	std::is_aggregate_v<T> &&       // Vertex must be an aggregate type, such as a plain struct.
	std::is_standard_layout_v<T> && // Vertex must have standard layout in order to be compatible with the layout expected by the shader.
	detail::is_vertex_attributes_v<decltype(reflection::fields(std::declval<T>()))>; // All vertex fields must be valid vertex attributes.

/**
 * Tag type for specifying that a Mesh does not have an index buffer.
 */
struct NoIndex {};

/**
 * Concept that checks if a type is a valid index type.
 *
 * \tparam T the type to check.
 */
template <typename T>
concept index =                         //
	std::is_same_v<T, NoIndex> ||       //
	std::is_same_v<T, std::uint8_t> ||  //
	std::is_same_v<T, std::uint16_t> || //
	std::is_same_v<T, std::uint32_t>;

/**
 * Tag type for specifying that a Mesh does not have an instance buffer.
 */
struct NoInstance {};

/**
 * Concept that checks if a type is a valid instance type.
 *
 * \tparam T the type to check.
 */
template <typename T>
concept instance =                  //
	std::is_aggregate_v<T> &&       // Instance must be an aggregate type, such as a plain struct.
	std::is_standard_layout_v<T> && // Instance must have standard layout in order to be compatible with the layout expected by the shader.
	detail::is_vertex_attributes_v<decltype(reflection::fields(std::declval<T>()))>; // All instance fields must be valid vertex attributes.

/**
 * Generic abstraction of a GPU vertex array object and its associated buffers.
 *
 * \tparam Vertex type of vertices stored in the vertex buffer. Must meet the
 *         requirements of the donut::graphics::vertex concept.
 * \tparam Index type of indices stored in the index buffer, or NoIndex for no
 *         index buffer. Must meet the requirements of the
 *         donut::graphics::index concept.
 * \tparam Instance type of instances stored in the instance buffer, or
 *         NoInstance for no instance buffer. Must meet the requirements of the
 *         donut::graphics::instance concept.
 */
template <vertex Vertex, index Index = NoIndex, instance Instance = NoInstance>
class Mesh {
public:
	/** Tells if the mesh has an index buffer or not. */
	static constexpr bool IS_INDEXED = !std::is_same_v<Index, NoIndex>;

	/** Tells if the mesh has an instance buffer or not. */
	static constexpr bool IS_INSTANCED = !std::is_same_v<Instance, NoInstance>;

	/**
	 * Constructor for meshes that only have a vertex buffer.
	 *
	 * \param verticesUsage intended access pattern of the vertex buffer, see
	 *        MeshBufferUsage.
	 * \param vertices initial data to copy into the vertex buffer.
	 */
	Mesh(MeshBufferUsage verticesUsage, std::span<const Vertex> vertices) requires(!IS_INDEXED && !IS_INSTANCED) {
		const detail::MeshStatePreserver preserver{};
		detail::bindVertexArray(vao.get());
		bufferVertexData(verticesUsage, vertices, 0);
	}

	/**
	 * Constructor for meshes that have a vertex buffer and an index buffer.
	 *
	 * \param verticesUsage intended access pattern of the vertex buffer, see
	 *        MeshBufferUsage.
	 * \param indicesUsage intended access pattern of the index buffer, see
	 *        MeshBufferUsage.
	 * \param vertices initial data to copy into the vertex buffer.
	 * \param indices initial data to copy into the index buffer.
	 */
	Mesh(MeshBufferUsage verticesUsage, MeshBufferUsage indicesUsage, std::span<const Vertex> vertices, std::span<const Index> indices) requires(IS_INDEXED && !IS_INSTANCED) {
		const detail::MeshStatePreserver preserver{};
		detail::bindVertexArray(vao.get());
		bufferVertexData(verticesUsage, vertices, 0);
		bufferIndexData(indicesUsage, indices);
	}

	/**
	 * Constructor for meshes that have a vertex buffer and an instance buffer.
	 *
	 * \param verticesUsage intended access pattern of the vertex buffer, see
	 *        MeshBufferUsage.
	 * \param instancesUsage intended access pattern of the instance buffer, see
	 *        MeshBufferUsage.
	 * \param vertices initial data to copy into the vertex buffer.
	 * \param instances initial data to copy into the instance buffer.
	 */
	Mesh(MeshBufferUsage verticesUsage, MeshBufferUsage instancesUsage, std::span<const Vertex> vertices, std::span<const Instance> instances) requires(
		!IS_INDEXED && IS_INSTANCED) {
		const detail::MeshStatePreserver preserver{};
		detail::bindVertexArray(vao.get());
		bufferVertexData(verticesUsage, vertices, 0);
		bufferInstanceData(instancesUsage, instances, static_cast<std::uint32_t>(reflection::aggregate_size_v<Vertex>));
	}

	/**
	 * Constructor for meshes that have a vertex buffer, an index buffer and an instance buffer.
	 *
	 * \param verticesUsage intended access pattern of the vertex buffer, see
	 *        MeshBufferUsage.
	 * \param indicesUsage intended access pattern of the index buffer, see
	 *        MeshBufferUsage.
	 * \param instancesUsage intended access pattern of the instance buffer, see
	 *        MeshBufferUsage.
	 * \param vertices initial data to copy into the vertex buffer.
	 * \param indices initial data to copy into the index buffer.
	 * \param instances initial data to copy into the instance buffer.
	 */
	Mesh(MeshBufferUsage verticesUsage, MeshBufferUsage indicesUsage, MeshBufferUsage instancesUsage, std::span<const Vertex> vertices, std::span<const Index> indices,
		std::span<const Instance> instances) requires(IS_INDEXED&& IS_INSTANCED) {
		const detail::MeshStatePreserver preserver{};
		detail::bindVertexArray(vao.get());
		bufferVertexData(verticesUsage, vertices, 0);
		bufferIndexData(indicesUsage, indices);
		bufferInstanceData(instancesUsage, instances, static_cast<std::uint32_t>(reflection::aggregate_size_v<Vertex>));
	}

	/**
	 * Set the contents of the vertex buffer.
	 *
	 * \param verticesUsage new intended access pattern of the vertex buffer,
	 *        see MeshBufferUsage.
	 * \param vertices new data to copy into the vertex buffer.
	 *
	 * \note The old contents of the buffer are discarded.
	 */
	void setVertices(MeshBufferUsage verticesUsage, std::span<const Vertex> vertices) noexcept requires(!IS_INDEXED) {
		const detail::MeshStatePreserver preserver{};
		detail::bindVertexArray(vao.get());
		detail::bindArrayBuffer(vbo.get());
		detail::bufferArrayBufferData(sizeof(Vertex) * vertices.size(), vertices.data(), verticesUsage);
	}

	/**
	 * Set the contents of the vertex and index buffers.
	 *
	 * \param verticesUsage new intended access pattern of the vertex buffer,
	 *        see MeshBufferUsage.
	 * \param indicesUsage new intended access pattern of the index buffer, see
	 *        MeshBufferUsage.
	 * \param vertices new data to copy into the vertex buffer.
	 * \param indices new data to copy into the index buffer.
	 *
	 * \note The old contents of the buffers are discarded.
	 */
	void setVertices(MeshBufferUsage verticesUsage, MeshBufferUsage indicesUsage, std::span<const Vertex> vertices, std::span<const Index> indices) noexcept requires(IS_INDEXED) {
		const detail::MeshStatePreserver preserver{};
		detail::bindVertexArray(vao.get());
		detail::bindArrayBuffer(vbo.get());
		detail::bufferArrayBufferData(sizeof(Vertex) * vertices.size(), vertices.data(), verticesUsage);
		detail::bindElementArrayBuffer(ebo.get());
		detail::bufferElementArrayBufferData(sizeof(Index) * indices.size(), indices.data(), indicesUsage);
	}

	/**
	 * Get an opaque handle to the GPU representation of the vertex buffer.
	 *
	 * \return a non-owning resource handle to the GPU representation of the
	 *         vertex buffer.
	 *
	 * \note This function is used internally by the implementations of various
	 *       abstractions and is not intended to be used outside of the graphics
	 *       module. The returned handle has no meaning to application code.
	 */
	[[nodiscard]] Handle getVertexBuffer() const noexcept {
		return vbo.get();
	}

	/**
	 * Get an opaque handle to the GPU representation of the index buffer.
	 *
	 * \return a non-owning resource handle to the GPU representation of the
	 *         index buffer.
	 *
	 * \note This function is used internally by the implementations of various
	 *       abstractions and is not intended to be used outside of the graphics
	 *       module. The returned handle has no meaning to application code.
	 */
	[[nodiscard]] Handle getIndexBuffer() const noexcept requires(IS_INDEXED) {
		return ebo.get();
	}

	/**
	 * Get an opaque handle to the GPU representation of the instance buffer.
	 *
	 * \return a non-owning resource handle to the GPU representation of the
	 *         instance buffer.
	 *
	 * \note This function is used internally by the implementations of various
	 *       abstractions and is not intended to be used outside of the graphics
	 *       module. The returned handle has no meaning to application code.
	 */
	[[nodiscard]] Handle getInstanceBuffer() const noexcept requires(IS_INSTANCED) {
		return ibo.get();
	}

	/**
	 * Get an opaque handle to the GPU representation of the vertex array.
	 *
	 * \return a non-owning resource handle to the GPU representation of the
	 *         vertex array.
	 *
	 * \note This function is used internally by the implementations of various
	 *       abstractions and is not intended to be used outside of the graphics
	 *       module. The returned handle has no meaning to application code.
	 */
	[[nodiscard]] Handle get() const noexcept {
		return vao.get();
	}

private:
	void bufferVertexData(MeshBufferUsage usage, std::span<const Vertex> vertices, std::uint32_t attributeOffset) {
		static_assert(std::is_aggregate_v<Vertex>, "Vertex type must be an aggregate type!");
		static_assert(std::is_standard_layout_v<Vertex>, "Vertex type must have standard layout!");
		detail::bindArrayBuffer(vbo.get());
		detail::bufferArrayBufferData(sizeof(Vertex) * vertices.size(), vertices.data(), usage);
		Vertex dummyVertex{};
		reflection::forEach(reflection::fields(dummyVertex), [&dummyVertex, &attributeOffset]<typename T>(T& dummyField) {
			const std::byte* const basePtr = reinterpret_cast<const std::byte*>(std::addressof(dummyVertex));
			const std::byte* const attributePtr = reinterpret_cast<const std::byte*>(std::addressof(dummyField));
			const std::uintptr_t offset = static_cast<std::uintptr_t>(attributePtr - basePtr);
			attributeOffset = detail::setupVertexAttribute<false, T>(attributeOffset, sizeof(Vertex), offset);
		});
	}

	void bufferIndexData(MeshBufferUsage usage, std::span<const Index> indices) requires(IS_INDEXED) {
		detail::bindElementArrayBuffer(ebo.get());
		detail::bufferElementArrayBufferData(sizeof(Index) * indices.size(), indices.data(), usage);
	}

	void bufferInstanceData(MeshBufferUsage usage, std::span<const Instance> instances, std::uint32_t attributeOffset) requires(IS_INSTANCED) {
		static_assert(std::is_aggregate_v<Instance>, "Instance type must be an aggregate type!");
		static_assert(std::is_standard_layout_v<Instance>, "Instance type must have standard layout!");
		detail::bindArrayBuffer(ibo.get());
		detail::bufferArrayBufferData(sizeof(Instance) * instances.size(), instances.data(), usage);
		Instance dummyInstance{};
		reflection::forEach(reflection::fields(dummyInstance), [&dummyInstance, &attributeOffset]<typename T>(T& dummyField) {
			const std::byte* const basePtr = reinterpret_cast<const std::byte*>(std::addressof(dummyInstance));
			const std::byte* const attributePtr = reinterpret_cast<const std::byte*>(std::addressof(dummyField));
			const std::uintptr_t offset = static_cast<std::uintptr_t>(attributePtr - basePtr);
			attributeOffset = detail::setupVertexAttribute<true, T>(attributeOffset, sizeof(Instance), offset);
		});
	}

	VertexArray vao{};
	Buffer vbo{};
	[[no_unique_address]] std::conditional_t<IS_INDEXED, Buffer, NoIndex> ebo{};
	[[no_unique_address]] std::conditional_t<IS_INSTANCED, Buffer, NoInstance> ibo{};
};

} // namespace graphics
} // namespace donut

#endif
