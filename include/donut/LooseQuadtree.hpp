#ifndef DONUT_LOOSE_QUADTREE_HPP
#define DONUT_LOOSE_QUADTREE_HPP

#include <donut/math.hpp>
#include <donut/shapes.hpp>

#include <algorithm>   // std::all_of
#include <array>       // std::array
#include <cassert>     // assert
#include <cstddef>     // std::size_t, std::ptrdiff_t
#include <cstdint>     // std::uint..._t
#include <iterator>    // std::iterator_traits
#include <optional>    // std::optional
#include <type_traits> // std::conditional_t, std::is_convertible_v, std::invoke_result_t
#include <utility>     // std::pair, std::forward, std::move, std::exchange
#include <vector>      // std::vector

namespace donut {

/**
 * Quadtree-based space subdivision container, optimized for intersection
 * queries between 2D axis-aligned boxes.
 *
 * \tparam T type of element to store in the tree.
 */
template <typename T>
class LooseQuadtree {
public:
	using value_type = T;
	using reference = T&;
	using const_reference = const T&;
	using pointer = T*;
	using const_pointer = const T*;
	using size_type = std::size_t;

private:
	using TreeIndex = std::uint32_t;
	using QuadrantIndexArray = std::array<TreeIndex, 4>;

	struct Quadrant {
		QuadrantIndexArray subQuadrantIndices{};
		TreeIndex parentIndex = 0;
		std::optional<T> element{};
	};

	template <bool Const>
	class Iterator {
	public:
		using reference = std::conditional_t<Const, LooseQuadtree::const_reference, LooseQuadtree::reference>;
		using pointer = std::conditional_t<Const, LooseQuadtree::const_pointer, LooseQuadtree::pointer>;
		using difference_type = std::ptrdiff_t;
		using value_type = LooseQuadtree::value_type;

		constexpr Iterator() noexcept = default;

		[[nodiscard]] constexpr reference operator*() const {
			assert(element);
			assert(*element);
			return **element;
		}

		[[nodiscard]] constexpr pointer operator->() const {
			return &**this;
		}

		[[nodiscard]] constexpr bool operator==(const Iterator& other) const noexcept {
			return element == other.element;
		}

		Iterator& operator++() {
			++element;
			return *this;
		}

		Iterator operator++(int) {
			Iterator old = *this;
			++*this;
			return old;
		}

	private:
		friend LooseQuadtree;

		Iterator(std::optional<T>* element, TreeIndex treeIndex) noexcept
			: element(element)
			, treeIndex(treeIndex) {}

		std::optional<T>* element = nullptr;
		TreeIndex treeIndex{};
	};

public:
	using iterator = Iterator<false>;
	using const_iterator = Iterator<true>;
	using difference_type = typename std::iterator_traits<iterator>::difference_type;

	/**
	 * Construct an empty tree.
	 *
	 * \param worldBoundingBox bounding box of the world, or the full region
	 *        that contains all other possible axis-aligned boxes that may be
	 *        inserted into the tree.
	 * \param typicalBoxSize minimum threshold for the size of a leaf quadrant.
	 *        This should correspond roughly to the typical size of the boxes
	 *        that will be inserted into the tree.
	 */
	LooseQuadtree(const Box<2, float>& worldBoundingBox, vec2 typicalBoxSize) noexcept {
		reset(worldBoundingBox, typicalBoxSize);
	}

	/**
	 * Reset the tree to an empty state with new world parameters.
	 *
	 * \param worldBoundingBox bounding box of the world, or the full region
	 *        that contains all other possible axis-aligned boxes that may be
	 *        inserted into the tree.
	 * \param typicalBoxSize minimum threshold for the size of a leaf quadrant.
	 *        This should correspond roughly to the typical size of the boxes
	 *        that will be inserted into the tree.
	 *
	 * \sa clear()
	 */
	void reset(const Box<2, float>& worldBoundingBox, vec2 typicalBoxSize) noexcept {
		clear();
		minimumQuadrantSize = max(typicalBoxSize.x, typicalBoxSize.y);
		rootCenter = (worldBoundingBox.min + worldBoundingBox.max) * 0.5f;
		const vec2 worldMaxExtents = max(worldBoundingBox.max - rootCenter, rootCenter - worldBoundingBox.min);
		const float worldMaxExtent = max(worldMaxExtents.x, worldMaxExtents.y);
		// Double the root size until it fits the entire world.
		halfRootSize = minimumQuadrantSize;
		while (halfRootSize < worldMaxExtent) {
			halfRootSize *= 2.0f;
		}
	}

	/**
	 * Erase all inserted elements from the tree.
	 *
	 * \sa reset()
	 * \sa erase()
	 */
	void clear() noexcept {
		tree.clear();
		firstFreeIndex = 0;
	}

	/**
	 * Try to construct a new element in the tree.
	 *
	 * \param elementBoundingBox axis-aligned bounding box of the element.
	 * \param args constructor arguments for the new element.
	 *
	 * \return a pair where:
	 *         - the first element contains an iterator to the newly inserted
	 *           element, or to the existing element if one was already
	 *           occupying the corresponding tree node, and
	 *         - the second element contains a bool that is true if an
	 *           element was successfully inserted, or false if an existing
	 *           element was already occupying the corresponding tree node.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the element constructor.
	 *
	 * \note To store multiple values in the same node of the tree, use a
	 *       list-like type for the element type T, such as std::vector,
	 *       std::forward_list or some intrusive linked list between the values.
	 *
	 * \sa insert()
	 * \sa operator[]()
	 */
	template <typename... Args>
	std::pair<iterator, bool> emplace(const Box<2, float>& elementBoundingBox, Args&&... args) {
		// Make sure the tree has a root.
		if (tree.empty()) {
			tree.emplace_back();
		}

		// Find the center of the AABB.
		const vec2 aabbDiagonal = elementBoundingBox.max - elementBoundingBox.min;
		const vec2 aabbCenter = elementBoundingBox.min + aabbDiagonal * 0.5f;

		// Find the largest extent of the AABB.
		const float aabbSize = max(aabbDiagonal.x, aabbDiagonal.y);

		// Start at the root of the tree and search for the smallest quadrant that contains the entire AABB within its loose bounds.
		// The loose bounds is a box around the quadrant that is twice as big as the quadrant in every direction and shares the same center.
		// Since the loose bounds of adjacent quadrants overlap, it could happen that the AABB is contained within multiple quadrants' loose bounds at the same time.
		// In that case, the closest quadrant, i.e. the one which contains the center of the AABB, is chosen.
		// The loop stops going lower in the tree when the AABB can no longer fit in a smaller quadrant, or when we reach the minimum quadrant size.
		float quadrantSize = halfRootSize;
		vec2 center = rootCenter;
		TreeIndex treeIndex = 0;
		while (quadrantSize >= aabbSize && quadrantSize >= minimumQuadrantSize) {
			quadrantSize *= 0.5f;

			// Determine which quadrant the AABB belongs to. This updates the center.
			const std::size_t quadrantIndexArrayIndex = chooseQuadrant(aabbCenter, center, quadrantSize);

			// Go to the quadrant.
			if (TreeIndex& quadrantIndex = tree[treeIndex].subQuadrantIndices[quadrantIndexArrayIndex]) {
				// The quadrant already exists in the tree. Go directly to it.
				treeIndex = quadrantIndex;
			} else {
				// The quadrant does not exist in the tree yet.
				// Acquire a space in the tree for the new quadrant.
				if (firstFreeIndex != 0) {
					// Re-use the first free quadrant.
					Quadrant& quadrant = tree[firstFreeIndex];
					// Set the new quadrant's parent index.
					quadrant.parentIndex = treeIndex;
					// Update the quadrant index to point to the new quadrant.
					quadrantIndex = firstFreeIndex;
					// Go to the new quadrant.
					treeIndex = firstFreeIndex;
					// Update the free index. The first sub-quadrant index in the quadrant leads to the next free quadrant.
					firstFreeIndex = std::exchange(quadrant.subQuadrantIndices.front(), 0);
				} else {
					// No free quadrants available for re-use. We need to allocate a new quadrant.
					// Save the new index. Don't change quadrantIndex before emplace_back() since the allocation might throw.
					const TreeIndex newQuadrantIndex = static_cast<TreeIndex>(tree.size());
					// Allocate the new quadrant.
					Quadrant& newQuadrant = tree.emplace_back();
					// Set the new quadrant's parent index.
					newQuadrant.parentIndex = treeIndex;
					// Update the quadrant index to point to the new quadrant.
					// Don't access through the quadrantIndex reference since it might have been invalidated by emplace_back().
					tree[treeIndex].subQuadrantIndices[quadrantIndexArrayIndex] = newQuadrantIndex;
					// Go to the new quadrant.
					treeIndex = newQuadrantIndex;
				}
			}
		}

		// Try to insert the new element into the selected quadrant.
		try {
			std::optional<T>& element = tree[treeIndex].element;
			if (element) {
				return {iterator{&element, treeIndex}, false};
			}
			element.emplace(std::forward<Args>(args)...);
			return {iterator{&element, treeIndex}, true};
		} catch (...) {
			cleanup(treeIndex);
			throw;
		}
	}

	/**
	 * Try to copy an element into the tree.
	 *
	 * \param elementBoundingBox axis-aligned bounding box of the element.
	 * \param value value to be copied into the tree.
	 *
	 * \return a pair where:
	 *         - the first element contains an iterator to the newly inserted
	 *           element, or to the existing element if one was already
	 *           occupying the corresponding tree node, and
	 *         - the second element contains a bool that is true if an
	 *           element was successfully inserted, or false if an existing
	 *           element was already occupying the corresponding tree node.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the element copy constructor.
	 *
	 * \note To store multiple values in the same node of the tree, use a
	 *       list-like type for the element type T, such as std::vector,
	 *       std::forward_list or some intrusive linked list between the values.
	 *
	 * \sa emplace()
	 * \sa operator[]()
	 */
	std::pair<iterator, bool> insert(const Box<2, float>& elementBoundingBox, const T& value) {
		return emplace(elementBoundingBox, value);
	}

	/**
	 * Try to move an element into the tree.
	 *
	 * \param elementBoundingBox axis-aligned bounding box of the element.
	 * \param value value to be moved into the tree.
	 *
	 * \return a pair where:
	 *         - the first element contains an iterator to the newly inserted
	 *           element, or to the existing element if one was already
	 *           occupying the corresponding tree node, and
	 *         - the second element contains a bool that is true if an
	 *           element was successfully inserted, or false if an existing
	 *           element was already occupying the corresponding tree node.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the element move constructor.
	 *
	 * \note To store multiple values in the same node of the tree, use a
	 *       list-like type for the element type T, such as std::vector,
	 *       std::forward_list or some intrusive linked list between the values.
	 *
	 * \sa emplace()
	 * \sa operator[]()
	 */
	std::pair<iterator, bool> insert(const Box<2, float>& elementBoundingBox, T&& value) {
		return emplace(elementBoundingBox, std::move(value));
	}

	/**
	 * Try to default-construct a new element in the tree and get a reference to
	 * it.
	 *
	 * \param elementBoundingBox axis-aligned bounding box of the element.
	 *
	 * \return a reference to the newly inserted element, or to the existing
	 *         element if one was already occupying the corresponding tree node.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the element default constructor.
	 *
	 * \note To store multiple values in the same node of the tree, use a
	 *       list-like type for the element type T, such as std::vector,
	 *       std::forward_list or some intrusive linked list between the values.
	 *
	 * \sa emplace()
	 * \sa insert()
	 */
	[[nodiscard]] T& operator[](const Box<2, float>& elementBoundingBox) {
		return *emplace(elementBoundingBox).first;
	}

	/**
	 * Remove an element from the tree.
	 *
	 * \param pos iterator to the element to remove. Must be valid.
	 *
	 * \sa clear()
	 */
	void erase(const_iterator pos) noexcept {
		assert(pos.element);
		pos.element->reset();
		cleanup(pos.treeIndex);
	}

	/**
	 * Execute a callback function for each active node of the tree, including
	 * empty branch nodes without an element.
	 *
	 * \param callback function to execute, which should accept the following
	 *        parameters (though they don't need to be used):
	 *        - `const donut::Box<2, float>& looseBounds`: an axis-aligned box
	 *          that defines the region that an element's bounding box must be
	 *          fully contained within in order to belong to the node.
	 *        - `const T* element`: a non-owning read-only pointer to the
	 *          element occupying the node, or nullptr if it does not have one.
	 *        .
	 *        The callback function should return either void or a bool that
	 *        specifies whether to stop the traversal or not. A value of true
	 *        means to stop and return early, while a value of false means to
	 *        continue traversing.
	 * \param predicate condition that must be met in order to traverse deeper
	 *        into the tree. Should accept the following parameter:
	 *        - `const donut::Box<2, float>& looseBounds`: an
	 *          axis-aligned box that defines the region that an element's
	 *          bounding box must be fully contained within in order to belong
	 *          to the next node.
	 *        .
	 *        The predicate function should return a bool that is true if the
	 *        next node should be traversed, or false if the branch should be
	 *        ignored.
	 *
	 * \return void if the callback function returns void, true if the callback
	 *         returns bool and exited early, false if the callback function
	 *         returns bool but didn't exit early.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the callback function or predicate
	 *         function.
	 *
	 * \note The order of traversal is unspecified, though it is guaranteed that
	 *       outer nodes will be visited before their own inner nodes that they
	 *       contain.
	 *
	 * \warning Although it is const, this function is not thread-safe since it
	 *          mutates an internal memory cache. Exclusive access is therefore
	 *          required for safety.
	 *
	 * \sa traverseElementNodes()
	 * \sa traverseElements()
	 * \sa test()
	 */
	template <typename Callback, typename Predicate>
	constexpr auto traverseActiveNodes(Callback&& callback, Predicate&& predicate) const { // NOLINT(cppcoreguidelines-missing-std-forward)
		return traverseNodesImpl(
			[callback = std::forward<Callback>(callback)](const Box<2, float>& looseBounds, const Quadrant& node) mutable {
				constexpr bool CALLBACK_RETURNS_BOOL = std::is_convertible_v<std::invoke_result_t<Callback, const Box<2, float>&, const T* const&>, bool>;
				const T* const element = (node.element) ? &*node.element : nullptr;
				if constexpr (CALLBACK_RETURNS_BOOL) {
					return callback(looseBounds, element);
				} else {
					callback(looseBounds, element);
				}
			},
			std::forward<Predicate>(predicate));
	}

	/**
	 * Execute a callback function for each active node of the tree, including
	 * empty branch nodes without an element.
	 *
	 * \param callback function to execute, which should accept the following
	 *        parameters (though they don't need to be used):
	 *        - `const donut::Box<2, float>& looseBounds`: an axis-aligned box
	 *          that defines the region that an element's bounding box must be
	 *          fully contained within in order to belong to the node.
	 *        - `const T* element`: a non-owning read-only pointer to the
	 *          element occupying the node, or nullptr if it does not have one.
	 *        .
	 *        The callback function should return either void or a bool that
	 *        specifies whether to stop the traversal or not. A value of true
	 *        means to stop and return early, while a value of false means to
	 *        continue traversing.
	 *
	 * \return void if the callback function returns void, true if the callback
	 *         returns bool and exited early, false if the callback function
	 *         returns bool but didn't exit early.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the callback function.
	 *
	 * \note The order of traversal is unspecified, though it is guaranteed that
	 *       outer nodes will be visited before their own inner nodes that they
	 *       contain.
	 *
	 * \warning Although it is const, this function is not thread-safe since it
	 *          mutates an internal memory cache. Exclusive access is therefore
	 *          required for safety.
	 *
	 * \sa traverseElementNodes()
	 * \sa traverseElements()
	 * \sa test()
	 */
	template <typename Callback>
	constexpr auto traverseActiveNodes(Callback&& callback) const {
		return traverseActiveNodes(std::forward<Callback>(callback), [](const Box<2, float>&) -> bool { return true; });
	}

	/**
	 * Execute a callback function for each active node of the tree that has an
	 * element.
	 *
	 * \param callback function to execute, which should accept the following
	 *        parameters (though they don't need to be used):
	 *        - `const donut::Box<2, float>& looseBounds`: an axis-aligned box
	 *          that defines the region that an element's bounding box must be
	 *          fully contained within in order to belong to the node.
	 *        - `const T& element`: a read-only reference to the element
	 *          occupying the node.
	 *        .
	 *        The callback function should return either void or a bool that
	 *        specifies whether to stop the traversal or not. A value of true
	 *        means to stop and return early, while a value of false means to
	 *        continue traversing.
	 * \param predicate condition that must be met in order to traverse deeper
	 *        into the tree. Should accept the following parameter:
	 *        - `const donut::Box<2, float>& looseBounds`: an
	 *          axis-aligned box that defines the region that an element's
	 *          bounding box must be fully contained within in order to belong
	 *          to the next node.
	 *        .
	 *        The predicate function should return a bool that is true if the
	 *        next node should be traversed, or false if the branch should be
	 *        ignored.
	 *
	 * \return void if the callback function returns void, true if the callback
	 *         returns bool and exited early, false if the callback function
	 *         returns bool but didn't exit early.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the callback function or predicate
	 *         function.
	 *
	 * \note The order of traversal is unspecified, though it is guaranteed that
	 *       outer nodes will be visited before their own inner nodes that they
	 *       contain.
	 *
	 * \warning Although it is const, this function is not thread-safe since it
	 *          mutates an internal memory cache. Exclusive access is therefore
	 *          required for safety.
	 *
	 * \sa traverseActiveNodes()
	 * \sa traverseElements()
	 * \sa test()
	 */
	template <typename Callback, typename Predicate>
	constexpr auto traverseElementNodes(Callback&& callback, Predicate&& predicate) const { // NOLINT(cppcoreguidelines-missing-std-forward)
		return traverseNodesImpl(
			[callback = std::forward<Callback>(callback)](const Box<2, float>& looseBounds, const Quadrant& node) mutable {
				constexpr bool CALLBACK_RETURNS_BOOL = std::is_convertible_v<std::invoke_result_t<Callback, const Box<2, float>&, const T&>, bool>;
				if (node.element) {
					if constexpr (CALLBACK_RETURNS_BOOL) {
						if (callback(looseBounds, *node.element)) {
							return true; // Callback requested early return.
						}
					} else {
						callback(looseBounds, *node.element);
					}
				}
				if constexpr (CALLBACK_RETURNS_BOOL) {
					return false;
				}
			},
			std::forward<Predicate>(predicate));
	}

	/**
	 * Execute a callback function for each active node of the tree that has an
	 * element.
	 *
	 * \param callback function to execute, which should accept the following
	 *        parameters (though they don't need to be used):
	 *        - `const donut::Box<2, float>& looseBounds`: an axis-aligned box
	 *          that defines the region that an element's bounding box must be
	 *          fully contained within in order to belong to the node.
	 *        - `const T& element`: a read-only reference to the element
	 *          occupying the node.
	 *        .
	 *        The callback function should return either void or a bool that
	 *        specifies whether to stop the traversal or not. A value of true
	 *        means to stop and return early, while a value of false means to
	 *        continue traversing.
	 *
	 * \return void if the callback function returns void, true if the callback
	 *         returns bool and exited early, false if the callback function
	 *         returns bool but didn't exit early.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the callback function.
	 *
	 * \note The order of traversal is unspecified, though it is guaranteed that
	 *       outer nodes will be visited before their own inner nodes that they
	 *       contain.
	 *
	 * \warning Although it is const, this function is not thread-safe since it
	 *          mutates an internal memory cache. Exclusive access is therefore
	 *          required for safety.
	 *
	 * \sa traverseActiveNodes()
	 * \sa traverseElements()
	 * \sa test()
	 */
	template <typename Callback>
	constexpr auto traverseElementNodes(Callback&& callback) const {
		return traverseElementNodes(std::forward<Callback>(callback), [](const Box<2, float>&) -> bool { return true; });
	}

	/**
	 * Execute a callback function for each element in the tree.
	 *
	 * \param callback function to execute, which should accept the following
	 *        parameter:
	 *        - `const T& element`: a read-only reference to the element.
	 *        .
	 *        The callback function should return either void or a bool that
	 *        specifies whether to stop the traversal or not. A value of true
	 *        means to stop and return early, while a value of false means to
	 *        continue traversing.
	 * \param predicate condition that must be met in order to traverse deeper
	 *        into the tree. Should accept the following parameter:
	 *        - `const donut::Box<2, float>& looseBounds`: an axis-aligned box
	 *          that defines the region that an element's bounding box must be
	 *          fully contained within in order to belong to the next node.
	 *        .
	 *        The predicate function should return a bool that is true if the
	 *        next node should be traversed, or false if the branch should be
	 *        ignored.
	 *
	 * \return void if the callback function returns void, true if the callback
	 *         returns bool and exited early, false if the callback function
	 *         returns bool but didn't exit early.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the callback function or predicate
	 *         function.
	 *
	 * \note The order of traversal is unspecified, though it is guaranteed that
	 *       outer nodes will be visited before their own inner nodes that they
	 *       contain.
	 *
	 * \warning Although it is const, this function is not thread-safe since it
	 *          mutates an internal memory cache. Exclusive access is therefore
	 *          required for safety.
	 *
	 * \sa traverseActiveNodes()
	 * \sa traverseElementNodes()
	 * \sa test()
	 */
	template <typename Callback, typename Predicate>
	constexpr auto traverseElements(Callback&& callback, Predicate&& predicate) const { // NOLINT(cppcoreguidelines-missing-std-forward)
		return traverseNodesImpl(
			[callback = std::forward<Callback>(callback)](const Box<2, float>&, const Quadrant& node) mutable {
				constexpr bool CALLBACK_RETURNS_BOOL = std::is_convertible_v<std::invoke_result_t<Callback, const T&>, bool>;
				if (node.element) {
					if constexpr (CALLBACK_RETURNS_BOOL) {
						if (callback(*node.element)) {
							return true; // Callback requested early return.
						}
					} else {
						callback(*node.element);
					}
				}
				if constexpr (CALLBACK_RETURNS_BOOL) {
					return false;
				}
			},
			std::forward<Predicate>(predicate));
	}

	/**
	 * Execute a callback function for each element in the tree.
	 *
	 * \param callback function to execute, which should accept the following
	 *        parameter:
	 *        - `const T& element`: a read-only reference to the element.
	 *        .
	 *        The callback function should return either void or a bool that
	 *        specifies whether to stop the traversal or not. A value of true
	 *        means to stop and return early, while a value of false means to
	 *        continue traversing.
	 *
	 * \return void if the callback function returns void, true if the callback
	 *         returns bool and exited early, false if the callback function
	 *         returns bool but didn't exit early.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the callback function.
	 *
	 * \note The order of traversal is unspecified, though it is guaranteed that
	 *       outer nodes will be visited before their own inner nodes that they
	 *       contain.
	 *
	 * \warning Although it is const, this function is not thread-safe since it
	 *          mutates an internal memory cache. Exclusive access is therefore
	 *          required for safety.
	 *
	 * \sa traverseActiveNodes()
	 * \sa traverseElementNodes()
	 * \sa test()
	 */
	template <typename Callback>
	constexpr auto traverseElements(Callback&& callback) const {
		return traverseElements(std::forward<Callback>(callback), [](const Box<2, float>&) -> bool { return true; });
	}

	/**
	 * Execute a callback function for each element in the tree that might
	 * contain a given point.
	 *
	 * \param point point to test.
	 * \param callback function to execute, which should accept the following
	 *        parameter:
	 *        - `const T& element`: a read-only reference to the element.
	 *        .
	 *        The callback function should return either void or a bool that
	 *        specifies whether to stop the traversal or not. A value of true
	 *        means to stop and return early, while a value of false means to
	 *        continue traversing.
	 *
	 * \return void if the callback function returns void, true if the callback
	 *         returns bool and exited early, false if the callback function
	 *         returns bool but didn't exit early.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the callback function.
	 *
	 * \note The order of traversal is unspecified, though it is guaranteed that
	 *       outer nodes will be visited before their own inner nodes that they
	 *       contain.
	 *
	 * \warning Although it is const, this function is not thread-safe since it
	 *          mutates an internal memory cache. Exclusive access is therefore
	 *          required for safety.
	 *
	 * \sa traverseActiveNodes()
	 * \sa traverseElementNodes()
	 * \sa traverseElements()
	 */
	template <typename Callback>
	auto test(vec2 point, Callback&& callback) const {
		return traverseElements(std::forward<Callback>(callback), [&point](const Box<2, float>& looseBounds) -> bool { return looseBounds.contains(point); });
	}

	/**
	 * Check if it is possible that some element in the tree contains a given
	 * point.
	 *
	 * \param point point to test.
	 *
	 * \return true if some element might contain the point, false otherwise.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \warning Although it is const, this function is not thread-safe since it
	 *          mutates an internal memory cache. Exclusive access is therefore
	 *          required for safety.
	 *
	 * \sa traverseActiveNodes()
	 * \sa traverseElementNodes()
	 * \sa traverseElements()
	 */
	[[nodiscard]] bool test(vec2 point) const noexcept {
		return traverseElements([](const T&) -> bool { return true; }, [&point](const Box<2, float>& looseBounds) -> bool { return looseBounds.contains(point); });
	}

	/**
	 * Execute a callback function for each element in the tree that might be
	 * intersecting with a given axis-aligned box.
	 *
	 * \param box box to test.
	 * \param callback function to execute, which should accept the following
	 *        parameter:
	 *        - `const T& element`: a read-only reference to the element.
	 *        .
	 *        The callback function should return either void or a bool that
	 *        specifies whether to stop the traversal or not. A value of true
	 *        means to stop and return early, while a value of false means to
	 *        continue traversing.
	 *
	 * \return void if the callback function returns void, true if the callback
	 *         returns bool and exited early, false if the callback function
	 *         returns bool but didn't exit early.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 * \throws any exception thrown by the callback function.
	 *
	 * \note The order of traversal is unspecified, though it is guaranteed that
	 *       outer nodes will be visited before their own inner nodes that they
	 *       contain.
	 *
	 * \warning Although it is const, this function is not thread-safe since it
	 *          mutates an internal memory cache. Exclusive access is therefore
	 *          required for safety.
	 *
	 * \sa traverseActiveNodes()
	 * \sa traverseElementNodes()
	 * \sa traverseElements()
	 */
	template <typename Callback>
	auto test(const Box<2, float>& box, Callback&& callback) const {
		return traverseElements(std::forward<Callback>(callback), [&box](const Box<2, float>& looseBounds) -> bool { return intersects(looseBounds, box); });
	}

	/**
	 * Check if it is possible that some element in the tree is intersecting
	 * with a given axis-aligned box.
	 *
	 * \param box box to test.
	 *
	 * \return true if some element might be intersecting with the box, false
	 *         otherwise.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \warning Although it is const, this function is not thread-safe since it
	 *          mutates an internal memory cache. Exclusive access is therefore
	 *          required for safety.
	 *
	 * \sa traverseActiveNodes()
	 * \sa traverseElementNodes()
	 * \sa traverseElements()
	 */
	[[nodiscard]] bool test(const Box<2, float>& box) const noexcept {
		return traverseElements([](const T&) -> bool { return true; }, [&box](const Box<2, float>& looseBounds) -> bool { return intersects(looseBounds, box); });
	}

private:
	struct IterationState {
		vec2 center;
		float quadrantSize;
		TreeIndex treeIndex;
	};

	[[nodiscard]] static constexpr std::size_t chooseQuadrant(vec2 aabbCenter, vec2& center, float halfQuadrantSize) {
		std::size_t index{};
		if (aabbCenter.x < center.x) {
			center.x -= halfQuadrantSize;
			if (aabbCenter.y < center.y) {
				center.y -= halfQuadrantSize;
				index = 0;
			} else {
				center.y += halfQuadrantSize;
				index = 1;
			}
		} else {
			center.x += halfQuadrantSize;
			if (aabbCenter.y < center.y) {
				center.y -= halfQuadrantSize;
				index = 2;
			} else {
				center.y += halfQuadrantSize;
				index = 3;
			}
		}
		return index;
	}

	template <typename Callback>
	static constexpr void forEachActiveQuadrant(const QuadrantIndexArray& subQuadrantIndices, vec2 center, float halfQuadrantSize,
		Callback&& callback) { // NOLINT(cppcoreguidelines-missing-std-forward)
		if (const TreeIndex quadrantIndex = subQuadrantIndices[0]) {
			callback(quadrantIndex, vec2{center.x - halfQuadrantSize, center.y - halfQuadrantSize});
		}
		if (const TreeIndex quadrantIndex = subQuadrantIndices[1]) {
			callback(quadrantIndex, vec2{center.x - halfQuadrantSize, center.y + halfQuadrantSize});
		}
		if (const TreeIndex quadrantIndex = subQuadrantIndices[2]) {
			callback(quadrantIndex, vec2{center.x + halfQuadrantSize, center.y - halfQuadrantSize});
		}
		if (const TreeIndex quadrantIndex = subQuadrantIndices[3]) {
			callback(quadrantIndex, vec2{center.x + halfQuadrantSize, center.y + halfQuadrantSize});
		}
	}

	template <typename Callback, typename Predicate>
	constexpr auto traverseNodesImpl(Callback&& callback, Predicate&& predicate) const { // NOLINT(cppcoreguidelines-missing-std-forward)
		constexpr bool CALLBACK_RETURNS_BOOL = std::is_convertible_v<std::invoke_result_t<Callback, const Box<2, float>&, const Quadrant&>, bool>;
		if (!tree.empty()) {
			iterationStack.clear();
			iterationStack.push_back(IterationState{
				.center = rootCenter,
				.quadrantSize = halfRootSize,
				.treeIndex = 0,
			});
			do {
				const auto [center, quadrantSize, treeIndex] = iterationStack.back();
				iterationStack.pop_back();

				const float size = quadrantSize * 2.0f;
				const Box<2, float> looseBounds{center - vec2{size}, center + vec2{size}};
				const Quadrant& node = tree[treeIndex];
				if constexpr (CALLBACK_RETURNS_BOOL) {
					if (callback(looseBounds, node)) {
						return true; // Callback requested early return.
					}
				} else {
					callback(looseBounds, node);
				}

				const float halfQuadrantSize = quadrantSize * 0.5f;
				forEachActiveQuadrant(node.subQuadrantIndices, center, halfQuadrantSize,
					[this, &predicate, quadrantSize = quadrantSize, halfQuadrantSize](TreeIndex quadrantIndex, vec2 quadrantCenter) {
						const Box<2, float> looseBounds{
							.min = quadrantCenter - vec2{quadrantSize},
							.max = quadrantCenter + vec2{quadrantSize},
						};
						if (predicate(looseBounds)) {
							iterationStack.push_back(IterationState{
								.center = quadrantCenter,
								.quadrantSize = halfQuadrantSize,
								.treeIndex = quadrantIndex,
							});
						}
					});
			} while (!iterationStack.empty());
		}
		if constexpr (CALLBACK_RETURNS_BOOL) {
			return false;
		}
	}

	void cleanup(TreeIndex treeIndex) noexcept {
		Quadrant* node = &tree[treeIndex];
		while (!node->element &&
			   std::all_of(node->subQuadrantIndices.begin(), node->subQuadrantIndices.end(), [](TreeIndex quadrantIndex) -> bool { return quadrantIndex == 0; })) {
			if (treeIndex == 0) {
				clear();
				break;
			}
			node->subQuadrantIndices.front() = firstFreeIndex;
			firstFreeIndex = treeIndex;
			treeIndex = node->parentIndex;
			node = &tree[treeIndex];
			for (TreeIndex& quadrantIndex : node->subQuadrantIndices) {
				if (quadrantIndex == firstFreeIndex) {
					quadrantIndex = 0;
					break;
				}
			}
		}
	}

	std::vector<Quadrant> tree{};
	float minimumQuadrantSize = 0.0f;
	float halfRootSize = 0.0f;
	vec2 rootCenter{0.0f, 0.0f};
	TreeIndex firstFreeIndex = 0;
	mutable std::vector<IterationState> iterationStack{};
};

} // namespace donut

#endif
