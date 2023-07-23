#ifndef DONUT_GRAPHICS_BUFFER_HPP
#define DONUT_GRAPHICS_BUFFER_HPP

#include <donut/UniqueHandle.hpp>
#include <donut/graphics/Handle.hpp>

namespace donut::graphics {

/**
 * Unique resource handle with exclusive ownership of a GPU memory buffer.
 */
class Buffer {
public:
	/**
	 * Create a new empty GPU memory buffer resource.
	 *
	 * \throws graphics::Error on failure to create the buffer object.
	 */
	Buffer();

	/**
	 * Get an opaque handle to the GPU representation of the buffer.
	 *
	 * \return a non-owning resource handle to the GPU representation of the
	 *         buffer.
	 *
	 * \note This function is used internally by the implementations of various
	 *       abstractions and is not intended to be used outside of the graphics
	 *       module. The returned handle has no meaning to application code.
	 */
	[[nodiscard]] Handle get() const noexcept {
		return buffer.get();
	}

private:
	struct BufferDeleter {
		void operator()(Handle handle) const noexcept;
	};

	UniqueHandle<Handle, BufferDeleter> buffer{};
};

} // namespace donut::graphics

#endif
