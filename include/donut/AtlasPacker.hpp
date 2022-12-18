#ifndef DONUT_ATLAS_PACKER_HPP
#define DONUT_ATLAS_PACKER_HPP

#include <cstddef> // std::size_t
#include <vector>  // std::vector

namespace donut {

template <std::size_t InitialResolution, std::size_t Padding>
class AtlasPacker {
public:
	static constexpr std::size_t INITIAL_RESOLUTION = InitialResolution;
	static constexpr std::size_t GROWTH_FACTOR = 2;
	static constexpr std::size_t PADDING = Padding;
	static constexpr float MINIMUM_ROW_HEIGHT_RATIO = 0.7f;

	struct InsertResult {
		std::size_t x;
		std::size_t y;
		bool resized;
	};

	[[nodiscard]] InsertResult insertRectangle(std::size_t width, std::size_t height) {
		const std::size_t paddedWidth = width + PADDING * std::size_t{2};
		const std::size_t paddedHeight = height + PADDING * std::size_t{2};

		Row* rowPtr = nullptr;
		for (Row& row : rows) {
			if (const float heightRatio = static_cast<float>(paddedHeight) / static_cast<float>(row.height);
				heightRatio >= MINIMUM_ROW_HEIGHT_RATIO && heightRatio <= 1.0f && paddedWidth <= resolution - row.width) {
				rowPtr = &row;
				break;
			}
		}

		bool resized = false;
		if (!rowPtr) {
			const std::size_t newRowTop = (rows.empty()) ? std::size_t{0} : rows.back().top + rows.back().height;
			const std::size_t newRowHeight = paddedHeight + paddedHeight / std::size_t{10};
			while (resolution < newRowTop + newRowHeight || resolution < paddedWidth) {
				resolution *= GROWTH_FACTOR;
				resized = true;
			}
			rowPtr = &rows.emplace_back(newRowTop, paddedHeight);
		}

		const std::size_t x = rowPtr->width + PADDING;
		const std::size_t y = rowPtr->top + PADDING;

		rowPtr->width += paddedWidth;

		return InsertResult{x, y, resized};
	}

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
