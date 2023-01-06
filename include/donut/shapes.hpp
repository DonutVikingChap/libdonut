#ifndef DONUT_SHAPES_HPP
#define DONUT_SHAPES_HPP

#include <glm/glm.hpp> // glm::...

namespace donut {

/**
 * Generic 2D circle shape with a center and radius.
 *
 * \tparam T component type for vector coordinates.
 */
template <typename T>
struct Circle {
	glm::vec<2, T> center; ///< Position of the center of the circle.
	T radius;              ///< Radius of the circle.

	/**
	 * Check if a given point is contained within the extents of this circle.
	 *
	 * \return true if the circle contains the given point, false otherwise.
	 */
	[[nodiscard]] constexpr bool contains(const glm::vec<2, T>& point) const noexcept {
		const glm::vec<2, T> difference = center - point;
		return glm::dot(difference, difference) < radius * radius;
	}
};

/**
 * Generic axis-aligned box shape with minimum and maximum extents.
 *
 * \tparam L number of vector dimensions.
 * \tparam T component type for vector coordinates.
 */
template <glm::length_t L, typename T>
struct AxisAlignedBox {
	glm::vec<L, T> min; ///< Position with the minimum coordinates of the box extents on each coordinate axis.
	glm::vec<L, T> max; ///< Position with the maximum coordinates of the box extents on each coordinate axis.

	/**
	 * Check if a given point is contained within the extents of this box.
	 *
	 * \return true if the box contains the given point, false otherwise.
	 */
	[[nodiscard]] constexpr bool contains(const glm::vec<L, T>& point) const noexcept {
		for (glm::length_t i = 0; i < L; ++i) {
			if (point[i] < min[i] || point[i] >= max[i]) {
				return false;
			}
		}
		return true;
	}
};

/**
 * Generic axis-aligned 2D rectangle shape with a position and size.
 *
 * \tparam T component type for vector coordinates.
 */
template <typename T>
struct Rectangle {
	glm::vec<2, T> position; ///< Position of the bottom left corner of the rectangle.
	glm::vec<2, T> size;     ///< Width and height of the rectangle.

	/**
	 * Check if a given point is contained within the extents of this rectangle.
	 *
	 * \return true if the rectangle contains the given point, false otherwise.
	 */
	[[nodiscard]] constexpr bool contains(const glm::vec<2, T>& point) const noexcept {
		return AxisAlignedBox<2, T>{position, position + size}.contains(point);
	}
};

/**
 * Check if two circles intersect.
 *
 * \param a first circle.
 * \param b second circle.
 *
 * \return true if the first and second circles are colliding with each other,
 *         false otherwise.
 */
template <typename T>
[[nodiscard]] constexpr bool intersects(const Circle<T>& a, const Circle<T>& b) noexcept {
	const glm::vec<2, T> difference = a.center - b.center;
	const T combinedRadius = a.radius + b.radius;
	return glm::dot(difference, difference) < combinedRadius * combinedRadius;
}

/**
 * Check if two axis-aligned boxes intersect.
 *
 * \param a first box.
 * \param b second box.
 *
 * \return true if the first and second boxes are colliding with each other,
 *         false otherwise.
 */
template <glm::length_t L, typename T>
[[nodiscard]] constexpr bool intersects(const AxisAlignedBox<L, T>& a, const AxisAlignedBox<L, T>& b) noexcept {
	for (glm::length_t i = 0; i < L; ++i) {
		if (a.min[i] >= b.max[i] || a.max[i] <= b.min[i]) {
			return false;
		}
	}
	return true;
}

/**
 * Check if two rectangles intersect.
 *
 * \param a first rectangle.
 * \param b second rectangle.
 *
 * \return true if the first and second rectangles are colliding with each
 *         other, false otherwise.
 */
template <typename T>
[[nodiscard]] constexpr bool intersects(const Rectangle<T>& a, const Rectangle<T>& b) noexcept {
	return intersects(AxisAlignedBox<2, T>{a.position, a.position + a.size}, AxisAlignedBox<2, T>{b.position, b.position + b.size});
}

} // namespace donut

#endif
