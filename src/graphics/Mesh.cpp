#include <donut/graphics/Handle.hpp>
#include <donut/graphics/Mesh.hpp>
#include <donut/graphics/opengl.hpp>

#include <cstddef> // std::size_t
#include <cstdint> // std::int32_t, std::uint32_t, std::uintptr_t

namespace donut {
namespace graphics {

namespace detail {

MeshStatePreserver::MeshStatePreserver() noexcept {
	GLint vertexArrayBinding = 0;
	GLint arrayBufferBinding = 0;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertexArrayBinding);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &arrayBufferBinding);
	vertexArrayBinding = static_cast<std::int32_t>(vertexArrayBinding);
	arrayBufferBinding = static_cast<std::int32_t>(arrayBufferBinding);
}

MeshStatePreserver::~MeshStatePreserver() {
	glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(arrayBufferBinding));
	glBindVertexArray(static_cast<GLuint>(vertexArrayBinding));
}

void bindVertexArray(Handle handle) {
	glBindVertexArray(handle);
}

void bindArrayBuffer(Handle handle) {
	glBindBuffer(GL_ARRAY_BUFFER, handle);
}

void bindElementArrayBuffer(Handle handle) {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
}

void enableVertexAttribArray(std::uint32_t index) {
	glEnableVertexAttribArray(static_cast<GLuint>(index));
}

void vertexAttribDivisor(std::uint32_t index, std::uint32_t divisor) {
	glVertexAttribDivisor(static_cast<GLuint>(index), static_cast<GLuint>(divisor));
}

void vertexAttribPointerUint(std::uint32_t index, std::size_t count, std::size_t stride, std::uintptr_t offset) {
	glVertexAttribIPointer(static_cast<GLuint>(index),
		static_cast<GLint>(count),
		GL_UNSIGNED_INT,
		static_cast<GLsizei>(stride),
		reinterpret_cast<const void*>(offset)); // NOLINT(performance-no-int-to-ptr)
}

void vertexAttribPointerFloat(std::uint32_t index, std::size_t count, std::size_t stride, std::uintptr_t offset) {
	glVertexAttribPointer(static_cast<GLuint>(index),
		static_cast<GLint>(count),
		GL_FLOAT,
		GL_FALSE,
		static_cast<GLsizei>(stride),
		reinterpret_cast<const void*>(offset)); // NOLINT(performance-no-int-to-ptr)
}

void bufferArrayBufferData(std::size_t size, const void* data, MeshBufferUsage usage) {
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), data, static_cast<GLenum>(usage));
}

void bufferElementArrayBufferData(std::size_t size, const void* data, MeshBufferUsage usage) {
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), data, static_cast<GLenum>(usage));
}

} // namespace detail

} // namespace graphics
} // namespace donut
