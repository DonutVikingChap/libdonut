#ifndef DONUT_ATLAS_PACKER_HPP
#define DONUT_ATLAS_PACKER_HPP

#include <cstddef> // std::size_t
#include <vector>  // std::vector

namespace donut {

/**
 * Axis-aligned rectangle packer for expandable square texture atlases.
 *
 * \tparam InitialResolution initial width of the square atlas region, in
 *         pixels.
 * \tparam Padding empty space to reserve between inserted rectangles, in
 *         pixels.
 */
template <std::size_t InitialResolution, std::size_t Padding>
class AtlasPacker {
public:
	/**
	 * The initial resolution that was passed to the InitialResolution template
	 * parameter.
	 */
	static constexpr std::size_t INITIAL_RESOLUTION = InitialResolution;

	/**
	 * The factor by which the resolution of the atlas will grow when it needs
	 * to make more space for a new rectangle.
	 */
	static constexpr std::size_t GROWTH_FACTOR = 2;

	/**
	 * The padding that was passed to the Padding template parameter.
	 */
	static constexpr std::size_t PADDING = Padding;

	/**
	 * The minimum ratio between the height of a new rectangle and the size of
	 * an existing row in the atlas for the new rectangle to be considered large
	 * enough to deserve a space in that row.
	 */
	static constexpr float MINIMUM_ROW_HEIGHT_RATIO = 0.7f;

	/**
	 * Result of the insertRectangle() function.
	 */
	struct InsertRectangleResult {
		/**
		 * The horizontal offset, in pixels, from the left edge of the atlas
		 * where the new rectangle was inserted.
		 */
		std::size_t x;

		/**
		 * The vertical offset, in pixels, from the bottom edge of the atlas
		 * where the new rectangle was inserted.
		 */
		std::size_t y;

		/**
		 * Whether the atlas needed to grow in order to accommodate the new
		 * rectangle or not. If true, the new required resolution can be queried
		 * by calling getResolution().
		 */
		bool resized;
	};

	/**
	 * Find and reserve a suitable space for a new axis-aligned rectangle to be
	 * inserted into the atlas.
	 *
	 * \param width width of the new rectangle, in pixels.
	 * \param width height of the new rectangle, in pixels.
	 *
	 * \return see InsertRectangleResult.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 */
	[[nodiscard]] InsertRectangleResult insertRectangle(std::size_t width, std::size_t height) {
		const std::size_t paddedWidth = width + PADDING * std::size_t{2};
		const std::size_t paddedHeight = height + PADDING * std::size_t{2};

		Row* rowPointer = nullptr;
		for (Row& row : rows) {
			if (const float heightRatio = static_cast<float>(paddedHeight) / static_cast<float>(row.height);
				heightRatio >= MINIMUM_ROW_HEIGHT_RATIO && heightRatio <= 1.0f && paddedWidth <= resolution - row.width) {
				rowPointer = &row;
				break;
			}
		}

		bool resized = false;
		if (!rowPointer) {
			const std::size_t newRowTop = (rows.empty()) ? std::size_t{0} : rows.back().top + rows.back().height;
			const std::size_t newRowHeight = paddedHeight + paddedHeight / std::size_t{10};
			while (resolution < newRowTop + newRowHeight || resolution < paddedWidth) {
				resolution *= GROWTH_FACTOR;
				resized = true;
			}
			rowPointer = &rows.emplace_back(newRowTop, paddedHeight);
		}

		const std::size_t x = rowPointer->width + PADDING;
		const std::size_t y = rowPointer->top + PADDING;

		rowPointer->width += paddedWidth;

		return InsertRectangleResult{x, y, resized};
	}

	/**
	 * Get the current required resolution of the atlas.
	 *
	 * \return the width of the square atlas region, in pixels.
	 */
	[[nodiscard]] std::size_t getResolution() const noexcept {
		return resolution;
	}

private:
	struct Row {
		Row(std::size_t top, std::size_t height) noexcept
			: top(top)
			, height(height) {}

		std::size_t top;
		std::size_t width = 0;
		std::size_t height;
	};

	std::vector<Row> rows{};
	std::size_t resolution = INITIAL_RESOLUTION;
};

} // namespace donut

#endif
