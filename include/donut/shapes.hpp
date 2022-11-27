#ifndef DONUT_SHAPES_HPP
#define DONUT_SHAPES_HPP

#include <glm/glm.hpp>

namespace donut {

template <typename T>
struct Circle {
	glm::vec<2, T> center;
	T radius;

	[[nodiscard]] constexpr bool contains(const glm::vec<2, T>& point) const noexcept {
		return square(center - point) < square(radius);
	}
};

template <glm::length_t L, typename T>
struct AxisAlignedBox {
	glm::vec<L, T> min;
	glm::vec<L, T> max;

	[[nodiscard]] constexpr bool contains(const glm::vec<L, T>& point) const noexcept {
		for (glm::length_t i = 0; i < L; ++i) {
			if (point[i] < min[i] || point[i] >= max[i]) {
				return false;
			}
		}
		return true;
	}
};

template <typename T>
struct Rectangle {
	glm::vec<2, T> position;
	glm::vec<2, T> size;

	[[nodiscard]] constexpr bool contains(const glm::vec<2, T>& point) const noexcept {
		return AxisAlignedBox<2, T>{position, position + size}.contains(point);
	}
};

template <typename T>
[[nodiscard]] constexpr bool intersects(const Circle<T>& a, const Circle<T>& b) noexcept {
	return square(a.center - b.center) < square(a.radius + b.radius);
}

template <glm::length_t L, typename T>
[[nodiscard]] constexpr bool intersects(const AxisAlignedBox<L, T>& a, const AxisAlignedBox<L, T>& b) noexcept {
	for (glm::length_t i = 0; i < L; ++i) {
		if (a.min[i] >= b.max[i] || a.max[i] <= b.min[i]) {
			return false;
		}
	}
	return true;
}

template <typename T>
[[nodiscard]] constexpr bool intersects(const Rectangle<T>& a, const Rectangle<T>& b) noexcept {
	return intersects(AxisAlignedBox<2, T>{a.position, a.position + a.size}, AxisAlignedBox<2, T>{b.position, b.position + b.size});
}

} // namespace donut

#endif
