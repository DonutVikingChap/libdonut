#ifndef DONUT_GRAPHICS_VERTEX_ARRAY_HPP
#define DONUT_GRAPHICS_VERTEX_ARRAY_HPP

#include <donut/Resource.hpp>
#include <donut/graphics/Handle.hpp>

namespace donut {
namespace graphics {

class VertexArray {
public:
	VertexArray();

	[[nodiscard]] Handle get() const noexcept;

private:
	struct VertexArrayDeleter {
		void operator()(Handle handle) const noexcept;
	};

	Resource<Handle, VertexArrayDeleter> vao{};
};

} // namespace graphics
} // namespace donut

#endif
