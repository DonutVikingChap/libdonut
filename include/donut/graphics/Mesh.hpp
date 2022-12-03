#ifndef DONUT_GRAPHICS_MESH_HPP
#define DONUT_GRAPHICS_MESH_HPP

#include <donut/graphics/Buffer.hpp>
#include <donut/graphics/Handle.hpp>
#include <donut/graphics/VertexArray.hpp>
#include <donut/reflection.hpp>

#include <cstddef>
#include <cstdint>
#include <glm/glm.hpp>
#include <memory>
#include <span>
#include <stdexcept>
#include <type_traits>

namespace donut {
namespace graphics {

enum class MeshBufferUsage : std::uint32_t {
	STATIC_COPY = 0x88E6,
	STATIC_DRAW = 0x88E4,
	STATIC_READ = 0x88E5,
	DYNAMIC_COPY = 0x88EA,
	DYNAMIC_DRAW = 0x88E8,
	DYNAMIC_READ = 0x88E9,
	STREAM_COPY = 0x88E2,
	STREAM_DRAW = 0x88E0,
	STREAM_READ = 0x88E1,
};

enum class MeshPrimitiveType : std::uint32_t {
	POINTS = 0x0000,
	LINES = 0x0001,
	LINE_LOOP = 0x0002,
	LINE_STRIP = 0x0003,
	TRIANGLES = 0x0004,
	TRIANGLE_STRIP = 0x0005,
};

enum class MeshIndexType : std::uint32_t {
	U8 = 0x1401,
	U16 = 0x1403,
	U32 = 0x1405,
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
inline void setupVertexAttribute(std::uint32_t& index, std::size_t stride, std::uintptr_t offset) {
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
}

} // namespace detail

struct NoIndex {};
struct NoInstance {};

template <typename Vertex, typename Index = NoIndex, typename Instance = NoInstance>
class Mesh {
public:
	static constexpr bool IS_INDEXED = !std::is_same_v<Index, NoIndex>;
	static constexpr bool IS_INSTANCED = !std::is_same_v<Instance, NoInstance>;

	Mesh(MeshBufferUsage verticesUsage, std::span<const Vertex> vertices) requires(!IS_INDEXED && !IS_INSTANCED) {
		const detail::MeshStatePreserver preserver{};
		detail::bindVertexArray(vao.get());
		bufferVertexData(verticesUsage, vertices, 0);
	}

	Mesh(MeshBufferUsage verticesUsage, MeshBufferUsage indicesUsage, std::span<const Vertex> vertices, std::span<const Index> indices) requires(IS_INDEXED && !IS_INSTANCED) {
		const detail::MeshStatePreserver preserver{};
		detail::bindVertexArray(vao.get());
		bufferVertexData(verticesUsage, vertices, 0);
		bufferIndexData(indicesUsage, indices);
	}

	Mesh(MeshBufferUsage verticesUsage, MeshBufferUsage instancesUsage, std::span<const Vertex> vertices, std::span<const Instance> instances) requires(
		!IS_INDEXED && IS_INSTANCED) {
		const detail::MeshStatePreserver preserver{};
		detail::bindVertexArray(vao.get());
		bufferVertexData(verticesUsage, vertices, 0);
		bufferInstanceData(instancesUsage, instances, static_cast<std::uint32_t>(reflection::aggregate_size_v<Vertex>));
	}

	Mesh(MeshBufferUsage verticesUsage, MeshBufferUsage indicesUsage, MeshBufferUsage instancesUsage, std::span<const Vertex> vertices, std::span<const Index> indices,
		std::span<const Instance> instances) requires(IS_INDEXED&& IS_INSTANCED) {
		const detail::MeshStatePreserver preserver{};
		detail::bindVertexArray(vao.get());
		bufferVertexData(verticesUsage, vertices, 0);
		bufferIndexData(indicesUsage, indices);
		bufferInstanceData(instancesUsage, instances, static_cast<std::uint32_t>(reflection::aggregate_size_v<Vertex>));
	}

	void setVertices(MeshBufferUsage verticesUsage, std::span<const Vertex> vertices) noexcept requires(!IS_INDEXED) {
		const detail::MeshStatePreserver preserver{};
		detail::bindVertexArray(vao.get());
		detail::bindArrayBuffer(vbo.get());
		detail::bufferArrayBufferData(sizeof(Vertex) * vertices.size(), vertices.data(), verticesUsage);
	}

	void setVertices(MeshBufferUsage verticesUsage, MeshBufferUsage indicesUsage, std::span<const Vertex> vertices, std::span<const Index> indices) noexcept requires(IS_INDEXED) {
		const detail::MeshStatePreserver preserver{};
		detail::bindVertexArray(vao.get());
		detail::bindArrayBuffer(vbo.get());
		detail::bufferArrayBufferData(sizeof(Vertex) * vertices.size(), vertices.data(), verticesUsage);
		detail::bindElementArrayBuffer(ebo.get());
		detail::bufferElementArrayBufferData(sizeof(Index) * indices.size(), indices.data(), indicesUsage);
	}

	[[nodiscard]] Handle getVertexBuffer() const noexcept {
		return vbo.get();
	}

	[[nodiscard]] Handle getIndexBuffer() const noexcept requires(IS_INDEXED) {
		return ebo.get();
	}

	[[nodiscard]] Handle getInstanceBuffer() const noexcept requires(IS_INSTANCED) {
		return ibo.get();
	}

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
			detail::setupVertexAttribute<false, T>(attributeOffset, sizeof(Vertex), offset);
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
			detail::setupVertexAttribute<true, T>(attributeOffset, sizeof(Instance), offset);
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
