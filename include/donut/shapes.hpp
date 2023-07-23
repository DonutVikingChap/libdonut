#ifndef DONUT_SHAPES_HPP
#define DONUT_SHAPES_HPP

#include <donut/math.hpp>

namespace donut {

/**
 * Generic point in space.
 *
 * \tparam L number of vector dimensions.
 * \tparam T component type for vector coordinates.
 */
template <length_t L, typename T>
using Point = vec<L, T>;

/**
 * Generic length in space.
 *
 * \tparam L number of vector dimensions.
 * \tparam T component type for vector coordinates.
 */
template <length_t L, typename T>
using Length = vec<L, T>;

/**
 * Generic line segment between two points.
 *
 * \tparam L number of vector dimensions.
 * \tparam T component type for vector coordinates.
 */
template <length_t L, typename T>
struct LineSegment {
	Point<L, T> pointA; ///< Position of the first point of the line segment.
	Point<L, T> pointB; ///< Position of the second point of the line segment.
};

/**
 * Generic sphere shape with a center and radius.
 *
 * \tparam L number of vector dimensions.
 * \tparam T component type for vector coordinates.
 */
template <length_t L, typename T>
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
template <length_t L, typename T>
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
template <length_t L, typename T>
struct Box {
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
	constexpr operator Box<2, T>() const noexcept {
		return Box<2, T>{.min = position, .max = position + size};
	}

	/**
	 * Check if a given point is contained within the extents of this rectangle.
	 *
	 * \return true if the rectangle contains the given point, false otherwise.
	 */
	[[nodiscard]] constexpr bool contains(const Point<2, T>& point) const noexcept;
};

/**
 * Get the axis-aligned bounding box of a line segment.
 *
 * \param line line segment to get the bounding box of.
 *
 * \return an axis-aligned box that contains the entire line segment.
 */
template <length_t L, typename T>
[[nodiscard]] constexpr Box<L, T> getAabbOf(const LineSegment<L, T>& line) noexcept {
	return {
		.min = min(line.pointA, line.pointB),
		.max = max(line.pointA, line.pointB),
	};
}

/**
 * Get the axis-aligned bounding box of a sphere.
 *
 * \param sphere sphere to get the bounding box of.
 *
 * \return an axis-aligned box that contains the entire sphere.
 */
template <length_t L, typename T>
[[nodiscard]] constexpr Box<L, T> getAabbOf(const Sphere<L, T>& sphere) noexcept {
	return {
		.min = sphere.center - Length<L, T>{sphere.radius},
		.max = sphere.center + Length<L, T>{sphere.radius},
	};
}

/**
 * Get the axis-aligned bounding box of a circle.
 *
 * \param circle circle to get the bounding box of.
 *
 * \return an axis-aligned box that contains the entire circle.
 */
template <typename T>
[[nodiscard]] constexpr Box<2, T> getAabbOf(const Circle<T>& circle) noexcept {
	return {
		.min = circle.center - Length<2, T>{circle.radius},
		.max = circle.center + Length<2, T>{circle.radius},
	};
}

/**
 * Get the axis-aligned bounding box of a capsule.
 *
 * \param capsule capsule to get the bounding box of.
 *
 * \return an axis-aligned box that contains the entire capsule.
 */
template <length_t L, typename T>
[[nodiscard]] constexpr Box<L, T> getAabbOf(const Capsule<L, T>& capsule) noexcept {
	return {
		.min = min(capsule.centerLine.pointA, capsule.centerLine.pointB) - Length<L, T>{capsule.radius},
		.max = max(capsule.centerLine.pointA, capsule.centerLine.pointB) + Length<L, T>{capsule.radius},
	};
}

/**
 * Get the axis-aligned bounding box of an axis-aligned box.
 *
 * \param box axis-aligned box to get the bounding box of.
 *
 * \return an axis-aligned box that contains the entire axis-aligned box.
 */
template <length_t L, typename T>
[[nodiscard]] constexpr Box<L, T> getAabbOf(const Box<L, T>& box) noexcept {
	return box;
}

/**
 * Get the axis-aligned bounding box of a rectangle.
 *
 * \param rectangle rectangle to get the bounding box of.
 *
 * \return an axis-aligned box that contains the entire rectangle.
 */
template <typename T>
[[nodiscard]] constexpr Box<2, T> getAabbOf(const Rectangle<T>& rectangle) noexcept {
	return static_cast<Box<2, T>>(rectangle);
}

/**
 * Check if two spheres intersect.
 *
 * \param a first sphere.
 * \param b second sphere.
 *
 * \return true if the first and second spheres are colliding with each other,
 *         false otherwise.
 */
template <length_t L, typename T>
[[nodiscard]] constexpr bool intersects(const Sphere<L, T>& a, const Sphere<L, T>& b) noexcept {
	return distance2(a.center, b.center) < length2(a.radius + b.radius);
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
template <length_t L, typename T>
[[nodiscard]] constexpr bool intersects(const Box<L, T>& a, const Box<L, T>& b) noexcept {
	for (length_t i = 0; i < L; ++i) {
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
	return intersects(static_cast<Box<2, T>>(a), static_cast<Box<2, T>>(b));
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
[[nodiscard]] constexpr bool intersects(const Rectangle<T>& a, const Box<2, T>& b) noexcept {
	return intersects(static_cast<Box<2, T>>(a), b);
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
[[nodiscard]] constexpr bool intersects(const Box<2, T>& a, const Rectangle<T>& b) noexcept {
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
template <length_t L, typename T>
[[nodiscard]] constexpr bool intersects(const Sphere<L, T>& a, const Box<L, T>& b) noexcept {
	return distance2(a.center, clamp(a.center, b.min, b.max)) < length2(a.radius);
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
template <length_t L, typename T>
[[nodiscard]] constexpr bool intersects(const Box<L, T>& a, const Sphere<L, T>& b) noexcept {
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
[[nodiscard]] constexpr bool intersects(const Circle<T>& a, const Box<2, T>& b) noexcept {
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
[[nodiscard]] constexpr bool intersects(const Box<2, T>& a, const Circle<T>& b) noexcept {
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
	return intersects(a, static_cast<Box<2, T>>(b));
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
	return intersects(static_cast<Sphere<2, T>>(a), static_cast<Box<2, T>>(b));
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
template <length_t L, typename T>
[[nodiscard]] constexpr bool intersects(const Sphere<L, T>& a, const Capsule<L, T>& b) noexcept {
	const T combinedRadiusSquared = length2(a.radius + b.radius);
	const vec<L, T> linePointAToPointB = b.centerLine.pointB - b.centerLine.pointA;
	const vec<L, T> linePointAToSphereCenter = a.center - b.centerLine.pointA;
	const T linePointAToSphereCenterAlongLine = dot(linePointAToSphereCenter, linePointAToPointB);
	if (linePointAToSphereCenterAlongLine <= 0.0f) {
		return length2(linePointAToSphereCenter) < combinedRadiusSquared;
	}
	const vec<L, T> linePointBToSphereCenter = a.center - b.centerLine.pointB;
	const T linePointBToSphereCenterAlongLine = dot(linePointBToSphereCenter, linePointAToPointB);
	if (linePointBToSphereCenterAlongLine >= 0.0f) {
		return length2(linePointBToSphereCenter) < combinedRadiusSquared;
	}
	const vec<L, T> lineToSphereCenterOrthogonal = linePointAToSphereCenter - linePointAToPointB * (linePointAToSphereCenterAlongLine / length2(linePointAToPointB));
	return length2(lineToSphereCenterOrthogonal) < combinedRadiusSquared;
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
template <length_t L, typename T>
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
template <length_t L, typename T>
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
template <length_t L, typename T>
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

template <length_t L, typename T>
constexpr bool Sphere<L, T>::contains(const Point<L, T>& point) const noexcept {
	return distance2(center, point) < length2(radius);
}

template <typename T>
constexpr bool Circle<T>::contains(const Point<2, T>& point) const noexcept {
	return static_cast<Sphere<2, T>>(*this).contains(point);
}

template <length_t L, typename T>
constexpr bool Capsule<L, T>::contains(const Point<L, T>& point) const noexcept {
	return intersects(*this, Sphere<L, T>{.center = point, .radius = T{0}});
}

template <length_t L, typename T>
constexpr bool Box<L, T>::contains(const Point<L, T>& point) const noexcept {
	for (length_t i = 0; i < L; ++i) {
		if (point[i] < min[i] || point[i] >= max[i]) {
			return false;
		}
	}
	return true;
}

template <typename T>
constexpr bool Rectangle<T>::contains(const Point<2, T>& point) const noexcept {
	return static_cast<Box<2, T>>(*this).contains(point);
}

} // namespace donut

#endif
