#include <donut/graphics/Buffer.hpp>
#include <donut/graphics/Error.hpp>
#include <donut/graphics/Handle.hpp>
#include <donut/graphics/opengl.hpp>

namespace donut {
namespace graphics {

Buffer::Buffer() {
	Handle handle{};
	glGenBuffers(1, &handle);
	if (!handle) {
		throw Error{"Failed to create buffer object!"};
	}
	buffer.reset(handle);
}

void Buffer::BufferDeleter::operator()(Handle handle) const noexcept {
	glDeleteBuffers(1, &handle);
}

} // namespace graphics
} // namespace donut
