#ifndef DONUT_GRAPHICS_VIEWPORT_HPP
#define DONUT_GRAPHICS_VIEWPORT_HPP

#include <glm/glm.hpp>

namespace donut {
namespace graphics {

struct Viewport {
	glm::ivec2 position;
	glm::ivec2 size;
};

} // namespace graphics
} // namespace donut

#endif
