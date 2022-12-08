#ifndef DONUT_GRAPHICS_VIEWPORT_HPP
#define DONUT_GRAPHICS_VIEWPORT_HPP

#include <glm/glm.hpp> // glm::...
#include <utility>     // std::pair

namespace donut {
namespace graphics {

struct Viewport {
	[[nodiscard]] static constexpr std::pair<Viewport, int> createIntegerScaled(glm::ivec2 windowSize, glm::ivec2 renderResolution) noexcept {
		Viewport result{.position{}, .size = renderResolution};
		int scale = 1;
		while (true) {
			const glm::ivec2 nextViewportSize = renderResolution * (scale + 1);
			if (nextViewportSize.x > windowSize.x || nextViewportSize.y > windowSize.y) {
				break;
			}
			result.size = nextViewportSize;
			++scale;
		}
		result.position = (windowSize - result.size) / 2;
		return {result, scale};
	}

	glm::ivec2 position;
	glm::ivec2 size;
};

} // namespace graphics
} // namespace donut

#endif
