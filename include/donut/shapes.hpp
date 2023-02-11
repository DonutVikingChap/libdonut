#ifndef DONUT_SHAPES_HPP
#define DONUT_SHAPES_HPP

#include <glm/glm.hpp>      // glm::...
#include <glm/gtx/norm.hpp> // glm::length2, glm::distance2

namespace donut {

/**
 * Generic point in space.
 *
 * \tparam L number of vector dimensions.
 * \tparam T component type for vector coordinates.
 */
template <glm::length_t L, typename T>
using Point = glm::vec<L, T>;

/**
 * Generic length in space.
 *
 * \tparam L number of vector dimensions.
 * \tparam T component type for vector coordinates.
 */
template <glm::length_t L, typename T>
using Length = glm::vec<L, T>;

/**
 * Generic line segment with a start position and an end position.
 *
 * \tparam L number of vector dimensions.
 * \tparam T component type for vector coordinates.
 */
template <glm::length_t L, typename T>
struct LineSegment {
	Point<L, T> start; ///< Position of the first point of the line segment.
	Point<L, T> end;   ///< Position of the second point of the line segment.
};

/**
 * Generic sphere shape with a center and radius.
 *
 * \tparam L number of vector dimensions.
 * \tparam T component type for vector coordinates.
 */
template <glm::length_t L, typename T>
struct Sphere {
	Point<L, T> center; ///< Position of the center of the sphere.
	T radius;           ///< Radius of the sphere.

	/**
	 * Check if a given point is contained within the extents of this sphere.
	 *
	 * \return true if the sphere contains the given point, false otherwise.
	 */
	[[nodiscard]] constexpr bool contains(const Point<L, T>& point) const noexcept;
};

/**
 * Flat 2D circle shape with a center and radius.
 *
 * \tparam T component type for vector coordinates.
 */
template <typename T>
struct Circle {
	Point<2, T> center; ///< Position of the center of the circle.
	T radius;           ///< Radius of the circle.

	/**
	 * Convert this circle to an equivalent 2D sphere.
	 *
	 * \return the circle as a sphere.
	 */
	constexpr operator Sphere<2, T>() const noexcept {
		return Sphere<2, T>{.center = center, .radius = radius};
	}

	/**
	 * Check if a given point is contained within the extents of this circle.
	 *
	 * \return true if the circle contains the given point, false otherwise.
	 */
	[[nodiscard]] constexpr bool contains(const Point<2, T>& point) const noexcept;
};

/**
 * Generic capsule shape with a center line segment and radius.
 *
 * \tparam L number of vector dimensions.
 * \tparam T component type for vector coordinates.
 */
template <glm::length_t L, typename T>
struct Capsule {
	LineSegment<L, T> centerLine; ///< Center line of the capsule.
	T radius;                     ///< Radius of the capsule from the center line.

	/**
	 * Check if a given point is contained within the extents of this capsule.
	 *
	 * \return true if the capsule contains the given point, false otherwise.
	 */
	[[nodiscard]] constexpr bool contains(const Point<L, T>& point) const noexcept;
};

/**
 * Generic axis-aligned box shape with minimum and maximum extents.
 *
 * \tparam L number of vector dimensions.
 * \tparam T component type for vector coordinates.
 */
template <glm::length_t L, typename T>
struct AxisAlignedBox {
	Point<L, T> min; ///< Position with the minimum coordinates of the box extents on each coordinate axis.
	Point<L, T> max; ///< Position with the maximum coordinates of the box extents on each coordinate axis.

	/**
	 * Check if a given point is contained within the extents of this box.
	 *
	 * \return true if the box contains the given point, false otherwise.
	 */
	[[nodiscard]] constexpr bool contains(const Point<L, T>& point) const noexcept;
};

/**
 * Flat 2D axis-aligned rectangle shape with a position and size.
 *
 * \tparam T component type for vector coordinates.
 */
template <typename T>
struct Rectangle {
	Point<2, T> position; ///< Position of the bottom left corner of the rectangle.
	Length<2, T> size;    ///< Width and height of the rectangle.

	/**
	 * Convert this rectangle to an equivalent 2D axis-aligned box.
	 *
	 * \return the rectangle as an axis-aligned box.
	 */
	constexpr operator AxisAlignedBox<2, T>() const noexcept {
		return AxisAlignedBox<2, T>{.min = position, .max = position + size};
	}

	/**
	 * Check if a given point is contained within the extents of this rectangle.
	 *
	 * \return true if the rectangle contains the given point, false otherwise.
	 */
	[[nodiscard]] constexpr bool contains(const Point<2, T>& point) const noexcept;
};

/**
 * Check if two spheres intersect.
 *
 * \param a first sphere.
 * \param b second sphere.
 *
 * \return true if the first and second spheres are colliding with each other,
 *         false otherwise.
 */
template <glm::length_t L, typename T>
[[nodiscard]] constexpr bool intersects(const Sphere<L, T>& a, const Sphere<L, T>& b) noexcept {
	return glm::distance2(a.center, b.center) < glm::length2(a.radius + b.radius);
}

/**
 * Check if two cirlces intersect.
 *
 * \param a first circle.
 * \param b second circle.
 *
 * \return true if the first and second circles are colliding with each other,
 *         false otherwise.
 */
template <typename T>
[[nodiscard]] constexpr bool intersects(const Circle<T>& a, const Circle<T>& b) noexcept {
	return intersects(static_cast<Sphere<2, T>>(a), static_cast<Sphere<2, T>>(b));
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
	return intersects(static_cast<AxisAlignedBox<2, T>>(a), static_cast<AxisAlignedBox<2, T>>(b));
}

/**
 * Check if a circle intersects with a sphere.
 *
 * \param a circle.
 * \param b sphere.
 *
 * \return true if the circle and sphere are colliding with each other, false
 *         otherwise.
 */
template <typename T>
[[nodiscard]] constexpr bool intersects(const Circle<T>& a, const Sphere<2, T>& b) noexcept {
	return intersects(static_cast<Sphere<2, T>>(a), b);
}

/**
 * Check if a sphere intersects with a circle.
 *
 * \param a sphere.
 * \param b circle.
 *
 * \return true if the sphere and circle are colliding with each other, false
 *         otherwise.
 */
template <typename T>
[[nodiscard]] constexpr bool intersects(const Sphere<2, T>& a, const Circle<T>& b) noexcept {
	return intersects(b, a);
}

/**
 * Check if a rectangle intersects with an axis-aligned box.
 *
 * \param a rectangle.
 * \param b box.
 *
 * \return true if the rectangle and box are colliding with each other, false
 *         otherwise.
 */
template <typename T>
[[nodiscard]] constexpr bool intersects(const Rectangle<T>& a, const AxisAlignedBox<2, T>& b) noexcept {
	return intersects(static_cast<AxisAlignedBox<2, T>>(a), b);
}

/**
 * Check if an axis-aligned box intersects with a rectangle.
 *
 * \param a box.
 * \param b rectangle.
 *
 * \return true if the box and rectangle are colliding with each other, false
 *         otherwise.
 */
template <typename T>
[[nodiscard]] constexpr bool intersects(const AxisAlignedBox<2, T>& a, const Rectangle<T>& b) noexcept {
	return intersects(b, a);
}

/**
 * Check if a sphere intersects with an axis-aligned box.
 *
 * \param a sphere.
 * \param b box.
 *
 * \return true if the sphere and box are colliding with each other, false
 *         otherwise.
 */
template <glm::length_t L, typename T>
[[nodiscard]] constexpr bool intersects(const Sphere<L, T>& a, const AxisAlignedBox<L, T>& b) noexcept {
	return glm::distance2(a.center, glm::clamp(a.center, b.min, b.max)) < glm::length2(a.radius);
}

/**
 * Check if an axis-aligned box intersects with a sphere.
 *
 * \param a box.
 * \param b sphere.
 *
 * \return true if the box and sphere are colliding with each other, false
 *         otherwise.
 */
template <glm::length_t L, typename T>
[[nodiscard]] constexpr bool intersects(const AxisAlignedBox<L, T>& a, const Sphere<L, T>& b) noexcept {
	return intersects(b, a);
}

/**
 * Check if a circle intersects with an axis-aligned box.
 *
 * \param a circle.
 * \param b box.
 *
 * \return true if the circle and box are colliding with each other, false
 *         otherwise.
 */
template <typename T>
[[nodiscard]] constexpr bool intersects(const Circle<T>& a, const AxisAlignedBox<2, T>& b) noexcept {
	return intersects(static_cast<Sphere<2, T>>(a), b);
}

/**
 * Check if an axis-aligned box intersects with a circle.
 *
 * \param a box.
 * \param b circle.
 *
 * \return true if the box and circle are colliding with each other, false
 *         otherwise.
 */
template <typename T>
[[nodiscard]] constexpr bool intersects(const AxisAlignedBox<2, T>& a, const Circle<T>& b) noexcept {
	return intersects(b, a);
}

/**
 * Check if a sphere intersects with a rectangle.
 *
 * \param a sphere.
 * \param b rectangle.
 *
 * \return true if the sphere and rectangle are colliding with each other, false
 *         otherwise.
 */
template <typename T>
[[nodiscard]] constexpr bool intersects(const Sphere<2, T>& a, const Rectangle<T>& b) noexcept {
	return intersects(a, static_cast<AxisAlignedBox<2, T>>(b));
}

/**
 * Check if a rectangle intersects with a sphere.
 *
 * \param a rectangle.
 * \param b sphere.
 *
 * \return true if the rectangle and sphere are colliding with each other, false
 *         otherwise.
 */
template <typename T>
[[nodiscard]] constexpr bool intersects(const Rectangle<T>& a, const Sphere<2, T>& b) noexcept {
	return intersects(b, a);
}

/**
 * Check if a circle intersects with a rectangle.
 *
 * \param a circle.
 * \param b rectangle.
 *
 * \return true if the circle and rectangle are colliding with each other, false
 *         otherwise.
 */
template <typename T>
[[nodiscard]] constexpr bool intersects(const Circle<T>& a, const Rectangle<T>& b) noexcept {
	return intersects(static_cast<Sphere<2, T>>(a), static_cast<AxisAlignedBox<2, T>>(b));
}

/**
 * Check if a rectangle intersects with a circle.
 *
 * \param a rectangle.
 * \param b circle.
 *
 * \return true if the rectangle and circle are colliding with each other, false
 *         otherwise.
 */
template <typename T>
[[nodiscard]] constexpr bool intersects(const Rectangle<T>& a, const Circle<T>& b) noexcept {
	return intersects(b, a);
}

/**
 * Check if a sphere intersects with a capsule.
 *
 * \param a sphere.
 * \param b capsule.
 *
 * \return true if the sphere and capsule are colliding with each other, false
 *         otherwise.
 */
template <glm::length_t L, typename T>
[[nodiscard]] constexpr bool intersects(const Sphere<L, T>& a, const Capsule<L, T>& b) noexcept {
	const glm::vec2 lineVector = b.centerLine.end - b.centerLine.start;
	const glm::vec2 lineStartToSphereCenter = a.center - b.centerLine.start;
	const float lineStartToSphereCenterAlongLine = glm::dot(lineStartToSphereCenter, lineVector);
	if (lineStartToSphereCenterAlongLine <= 0.0f) {
		return glm::length2(lineStartToSphereCenter) < glm::length2(a.radius + b.radius);
	}
	const glm::vec2 lineEndToSphereCenter = a.center - b.centerLine.end;
	const float lineEndToSphereCenterAlongLine = glm::dot(lineEndToSphereCenter, lineVector);
	if (lineEndToSphereCenterAlongLine >= 0.0f) {
		return glm::length2(lineEndToSphereCenter) < glm::length2(a.radius + b.radius);
	}
	const glm::vec2 lineToSphereCenterOrthogonal = lineStartToSphereCenter - lineVector * (lineStartToSphereCenterAlongLine / glm::length2(lineVector));
	return glm::length2(lineToSphereCenterOrthogonal) < glm::length2(a.radius + b.radius);
}

/**
 * Check if a capsule intersects with a sphere.
 *
 * \param a capsule.
 * \param b sphere.
 *
 * \return true if the capsule and sphere are colliding with each other, false
 *         otherwise.
 */
template <glm::length_t L, typename T>
[[nodiscard]] constexpr bool intersects(const Capsule<L, T>& a, const Sphere<L, T>& b) noexcept {
	return intersects(b, a);
}

/**
 * Check if a circle intersects with a capsule.
 *
 * \param a circle.
 * \param b capsule.
 *
 * \return true if the circle and capsule are colliding with each other, false
 *         otherwise.
 */
template <typename T>
[[nodiscard]] constexpr bool intersects(const Circle<T>& a, const Capsule<2, T>& b) noexcept {
	return intersects(static_cast<Sphere<2, T>>(a), b);
}

/**
 * Check if a capsule intersects with a circle.
 *
 * \param a capsule.
 * \param b circle.
 *
 * \return true if the capsule and circle are colliding with each other, false
 *         otherwise.
 */
template <typename T>
[[nodiscard]] constexpr bool intersects(const Capsule<2, T>& a, const Circle<T>& b) noexcept {
	return intersects(b, a);
}

/**
 * Check if a sphere intersects with a line segment.
 *
 * \param a sphere.
 * \param b line segment.
 *
 * \return true if the sphere and line segment are colliding with each other,
 *         false otherwise.
 */
template <glm::length_t L, typename T>
[[nodiscard]] constexpr bool intersects(const Sphere<L, T>& a, const LineSegment<L, T>& b) noexcept {
	return intersects(a, Capsule<L, T>{.centerLine = b, .radius = T{0}});
}

/**
 * Check if a line segment intersects with a sphere.
 *
 * \param a line segment.
 * \param b sphere.
 *
 * \return true if the line segment and sphere are colliding with each other,
 *         false otherwise.
 */
template <glm::length_t L, typename T>
[[nodiscard]] constexpr bool intersects(const LineSegment<L, T>& a, const Sphere<L, T>& b) noexcept {
	return intersects(b, a);
}

/**
 * Check if a circle intersects with a line segment.
 *
 * \param a circle.
 * \param b line segment.
 *
 * \return true if the circle and line segment are colliding with each other,
 *         false otherwise.
 */
template <typename T>
[[nodiscard]] constexpr bool intersects(const Circle<T>& a, const LineSegment<2, T>& b) noexcept {
	return intersects(static_cast<Sphere<2, T>>(a), b);
}

/**
 * Check if a line segment intersects with a circle.
 *
 * \param a line segment.
 * \param b circle.
 *
 * \return true if the line segment and circle are colliding with each other,
 *         false otherwise.
 */
template <typename T>
[[nodiscard]] constexpr bool intersects(const LineSegment<2, T>& a, const Circle<T>& b) noexcept {
	return intersects(b, a);
}

template <glm::length_t L, typename T>
constexpr bool Sphere<L, T>::contains(const Point<L, T>& point) const noexcept {
	return glm::distance2(center, point) < glm::length2(radius);
}

template <typename T>
constexpr bool Circle<T>::contains(const Point<2, T>& point) const noexcept {
	return static_cast<Sphere<2, T>>(*this).contains(point);
}

template <glm::length_t L, typename T>
constexpr bool Capsule<L, T>::contains(const Point<L, T>& point) const noexcept {
	return intersects(*this, Sphere<L, T>{.center = point, .radius = T{0}});
}

template <glm::length_t L, typename T>
constexpr bool AxisAlignedBox<L, T>::contains(const Point<L, T>& point) const noexcept {
	for (glm::length_t i = 0; i < L; ++i) {
		if (point[i] < min[i] || point[i] >= max[i]) {
			return false;
		}
	}
	return true;
}

template <typename T>
constexpr bool Rectangle<T>::contains(const Point<2, T>& point) const noexcept {
	return static_cast<AxisAlignedBox<2, T>>(*this).contains(point);
}

} // namespace donut

#endif
