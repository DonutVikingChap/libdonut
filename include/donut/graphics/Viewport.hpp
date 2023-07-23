#ifndef DONUT_GRAPHICS_VIEWPORT_HPP
#define DONUT_GRAPHICS_VIEWPORT_HPP

#include <donut/math.hpp>

#include <utility> // std::pair

namespace donut::graphics {

/**
 * Rectangular region of a framebuffer.
 */
struct Viewport {
	/**
	 * Create an integer-scaled viewport that fits into the middle of a
	 * framebuffer at the largest positive integer scale of an original render
	 * resolution that still fits within the framebuffer.
	 *
	 * \param framebufferSize the size of the framebuffer to fit the viewport
	 *        into, in pixels.
	 * \param renderResolution the original rendered size to be scaled into the
	 *        framebuffer, in pixels.
	 *
	 * \return a pair where:
	 *         - the first element contains the new scaled viewport, and
	 *         - the second element contains the integer scale that was chosen.
	 *
	 * \note If the original render resolution cannot fit within the
	 *       framebuffer, then a viewport with the original render resolution
	 *       will be returned along with a scale of 1.
	 */
	[[nodiscard]] static constexpr std::pair<Viewport, int> createIntegerScaled(ivec2 framebufferSize, ivec2 renderResolution) noexcept {
		Viewport result{.position{}, .size = renderResolution};
		int scale = 1;
		while (true) {
			const ivec2 nextViewportSize = renderResolution * (scale + 1);
			if (nextViewportSize.x > framebufferSize.x || nextViewportSize.y > framebufferSize.y) {
				break;
			}
			result.size = nextViewportSize;
			++scale;
		}
		result.position = (framebufferSize - result.size) / 2;
		return {result, scale};
	}

	/**
	 * The offset of the viewport, in pixels, from the bottom left of the
	 * framebuffer.
	 */
	ivec2 position;

	/**
	 * The width and height of the viewport, in pixels.
	 */
	ivec2 size;
};

} // namespace donut::graphics

#endif
