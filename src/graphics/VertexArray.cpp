#include <donut/graphics/Error.hpp>
#include <donut/graphics/Handle.hpp>
#include <donut/graphics/VertexArray.hpp>
#include <donut/graphics/opengl.hpp>

namespace donut {
namespace graphics {

VertexArray::VertexArray() {
	Handle handle{};
	glGenVertexArrays(1, &handle);
	if (!handle) {
		throw Error{"Failed to create vertex array object!"};
	}
	vao.reset(handle);
}

Handle VertexArray::get() const noexcept {
	return vao.get();
}

void VertexArray::VertexArrayDeleter::operator()(Handle handle) const noexcept {
	glDeleteVertexArrays(1, &handle);
}

} // namespace graphics
} // namespace donut
