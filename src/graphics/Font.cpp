#include <donut/Color.hpp>
#include <donut/InputFileStream.hpp>
#include <donut/graphics/Error.hpp>
#include <donut/graphics/Font.hpp>
#include <donut/graphics/Renderer.hpp>
#include <donut/graphics/Texture.hpp>
#include <donut/unicode.hpp>

#include <algorithm>    // std::min, std::max
#include <cmath>        // std::floor, std::round
#include <cstddef>      // std::size_t, std::byte
#include <cstdint>      // std::uint32_t
#include <fmt/format.h> // fmt::format
#include <glm/glm.hpp>  // glm::...
#include <schrift.h>    // SFT..., sft_...
#include <string_view>  // std::string_view, std::u8string_view
#include <vector>       // std::vector

namespace donut {
namespace graphics {

Font::Font(const char* filepath, const FontOptions& options)
	: fontFileContents(InputFileStream::open(filepath).readAll())
	, font(sft_loadmem(fontFileContents.data(), fontFileContents.size()))
	, options(options) {
	if (!font) {
		throw Error{fmt::format("Failed to load font \"{}\".", filepath)};
	}
}

const Font::Glyph* Font::findGlyph(std::uint32_t characterSize, char32_t codePoint) const noexcept {
	if (const auto it = glyphs.find(makeGlyphKey(characterSize, codePoint)); it != glyphs.end()) {
		return &it->second;
	}
	return nullptr;
}

const Font::Glyph& Font::loadGlyph(Renderer& renderer, std::uint32_t characterSize, char32_t codePoint) {
	const auto [it, inserted] = glyphs.try_emplace(makeGlyphKey(characterSize, codePoint));
	if (inserted) {
		it->second = renderGlyph(renderer, characterSize, codePoint);
	}
	return it->second;
}

Font::ShapedText Font::shapeText(Renderer& renderer, std::uint32_t characterSize, std::u8string_view string, glm::vec2 scale) {
	ShapedText result{
		.shapedGlyphs{},
		.extentsMin{0.0f, 0.0f},
		.extentsMax{0.0f, 0.0f},
		.rowCount = 1,
	};
	glm::vec2 offset{0.0f, 0.0f};
	const float xBegin = offset.x;
	const unicode::UTF8View codePoints{string};
	for (auto it = codePoints.begin(); it != codePoints.end();) {
		if (const char32_t codePoint = *it++; codePoint == '\n') {
			offset.x = xBegin;
			offset.y -= std::floor(getLineMetrics(characterSize).height * scale.y);
			++result.rowCount;
		} else {
			const Glyph& glyph = loadGlyph(renderer, characterSize, codePoint);
			const glm::vec2 glyphOffset{
				std::floor(offset.x + std::round(glyph.bearing.x * scale.x)),
				std::floor(offset.y + std::round(glyph.bearing.y * scale.y)),
			};
			const glm::vec2 glyphSize{
				std::round(glyph.size.x * scale.x),
				std::round(glyph.size.y * scale.y),
			};
			result.shapedGlyphs.push_back({
				.offset = glyphOffset,
				.size = glyphSize,
				.textureOffset = glyph.textureOffset,
				.textureScale = glyph.textureScale,
			});
			result.extentsMin.x = std::min(result.extentsMin.x, glyphOffset.x);
			result.extentsMin.y = std::min(result.extentsMin.y, glyphOffset.y);
			result.extentsMax.x = std::max(result.extentsMax.x, glyphOffset.x + glyphSize.x);
			result.extentsMax.y = std::max(result.extentsMax.y, glyphOffset.y + glyphSize.y);
			const glm::vec2 kerning = getKerning(characterSize, codePoint, (it == codePoints.end()) ? char32_t{0} : *it);
			offset += glm::vec2{glyph.advance + kerning.x, kerning.y} * scale;
		}
	}
	return result;
}

Font::ShapedText Font::shapeText(Renderer& renderer, std::uint32_t characterSize, std::string_view string, glm::vec2 scale) {
	static_assert(sizeof(char) == sizeof(char8_t));
	static_assert(alignof(char) == alignof(char8_t));
	return shapeText(renderer, characterSize, std::u8string_view{reinterpret_cast<const char8_t*>(string.data()), string.size()}, scale);
}

Font::LineMetrics Font::getLineMetrics(std::uint32_t characterSize) const noexcept {
	const SFT sft{
		.font = static_cast<SFT_Font*>(font.get()),
		.xScale = static_cast<double>(characterSize),
		.yScale = static_cast<double>(characterSize),
		.xOffset = 0.0,
		.yOffset = 0.0,
		.flags = 0,
	};

	SFT_LMetrics lmetrics{};
	if (sft_lmetrics(&sft, &lmetrics) != 0) {
		return {};
	}

	return {
		.ascender = static_cast<float>(lmetrics.ascender),
		.descender = static_cast<float>(lmetrics.descender),
		.height = static_cast<float>(lmetrics.ascender + lmetrics.lineGap),
	};
}

glm::vec2 Font::getKerning(std::uint32_t characterSize, char32_t left, char32_t right) const noexcept {
	const SFT sft{
		.font = static_cast<SFT_Font*>(font.get()),
		.xScale = static_cast<double>(characterSize),
		.yScale = static_cast<double>(characterSize),
		.xOffset = 0.0,
		.yOffset = 0.0,
		.flags = 0,
	};

	SFT_Glyph leftGlyph{};
	if (sft_lookup(&sft, SFT_UChar{left}, &leftGlyph) != 0) {
		return {};
	}

	SFT_Glyph rightGlyph{};
	if (sft_lookup(&sft, SFT_UChar{right}, &rightGlyph) != 0) {
		return {};
	}

	SFT_Kerning kerning{};
	if (sft_kerning(&sft, leftGlyph, rightGlyph, &kerning) != 0) {
		return {};
	}

	return {static_cast<float>(kerning.xShift), static_cast<float>(kerning.yShift)};
}

void Font::prepareAtlasTexture(Renderer& renderer, bool resized) {
	if (atlasTexture) {
		if (resized) {
			atlasTexture.grow2D(renderer, atlasPacker.getResolution(), atlasPacker.getResolution(), Color::INVISIBLE);
			const glm::vec2 textureSize = atlasTexture.getSize2D();
			for (auto& [codePoint, glyph] : glyphs) {
				glyph.textureOffset = glyph.position / textureSize;
				glyph.textureScale = glyph.size / textureSize;
			}
		}
	} else {
		atlasTexture = {
			TextureFormat::R8_UNORM,
			atlasPacker.getResolution(),
			atlasPacker.getResolution(),
			{.repeat = false, .useLinearFiltering = options.useLinearFiltering, .useMipmap = false},
		};
		atlasTexture.fill2D(renderer, Color::INVISIBLE);
	}
}

Font::Glyph Font::renderGlyph(Renderer& renderer, std::uint32_t characterSize, char32_t codePoint) {
	const SFT sft{
		.font = static_cast<SFT_Font*>(font.get()),
		.xScale = static_cast<double>(characterSize),
		.yScale = static_cast<double>(characterSize),
		.xOffset = 0.0,
		.yOffset = 0.0,
		.flags = 0,
	};

	SFT_Glyph glyph{};
	if (sft_lookup(&sft, SFT_UChar{codePoint}, &glyph) != 0) {
		throw Error{fmt::format("Failed to lookup font glyph for code point U+{:04X}", static_cast<std::uint32_t>(codePoint))};
	}

	SFT_GMetrics gmetrics{};
	sft_gmetrics(&sft, glyph, &gmetrics);

	const std::size_t width = static_cast<std::size_t>(gmetrics.minWidth);
	const std::size_t height = static_cast<std::size_t>(gmetrics.minHeight);
	const auto [x, y, resized] = atlasPacker.insertRectangle(width, height);
	prepareAtlasTexture(renderer, resized);

	if (width > 0 && height > 0) {
		std::vector<std::byte> pixels(width * height);
		if (sft_render(&sft, glyph, SFT_Image{.pixels = pixels.data(), .width = static_cast<int>(width), .height = static_cast<int>(height)}) != 0) {
			throw Error{fmt::format("Failed to render font glyph for code point U+{:04X}", static_cast<std::uint32_t>(codePoint))};
		}
		atlasTexture.pasteImage2D(width, height, PixelFormat::R, PixelComponentType::U8, pixels.data(), x, y);
	}

	const glm::vec2 textureSize = atlasTexture.getSize2D();
	const glm::vec2 position{static_cast<float>(x), static_cast<float>(y)};
	const glm::vec2 size{static_cast<float>(width), static_cast<float>(height)};
	const glm::vec2 bearing{static_cast<float>(gmetrics.leftSideBearing), static_cast<float>(gmetrics.yOffset)};
	const float advance = static_cast<float>(gmetrics.advanceWidth);
	return {
		.textureOffset = position / textureSize,
		.textureScale = size / textureSize,
		.position = position,
		.size = size,
		.bearing = bearing,
		.advance = advance,
	};
}

void Font::FontDeleter::operator()(void* handle) const noexcept {
	sft_freefont(static_cast<SFT_Font*>(handle));
}

} // namespace graphics
} // namespace donut
