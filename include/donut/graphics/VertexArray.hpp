#ifndef DONUT_GRAPHICS_VERTEX_ARRAY_HPP
#define DONUT_GRAPHICS_VERTEX_ARRAY_HPP

#include <donut/UniqueHandle.hpp>
#include <donut/graphics/Handle.hpp>

namespace donut::graphics {

/**
 * Unique resource handle with exclusive ownership of a GPU vertex array.
 */
class VertexArray {
public:
	/**
	 * Create a new empty GPU vertex array resource.
	 *
	 * \throws graphics::Error on failure to create the vertex array object.
	 */
	VertexArray();

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
	[[nodiscard]] Handle get() const noexcept;

private:
	struct VertexArrayDeleter {
		void operator()(Handle handle) const noexcept;
	};

	UniqueHandle<Handle, VertexArrayDeleter> vao{};
};

} // namespace donut::graphics

#endif
