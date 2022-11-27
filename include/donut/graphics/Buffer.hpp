#ifndef DONUT_GRAPHICS_BUFFER_HPP
#define DONUT_GRAPHICS_BUFFER_HPP

#include <donut/Resource.hpp>
#include <donut/graphics/Handle.hpp>

namespace donut {
namespace graphics {

class Buffer {
public:
	Buffer();

	[[nodiscard]] Handle get() const noexcept {
		return buffer.get();
	}

private:
	struct BufferDeleter {
		void operator()(Handle handle) const noexcept;
	};

	Resource<Handle, BufferDeleter> buffer{};
};

} // namespace graphics
} // namespace donut

#endif
