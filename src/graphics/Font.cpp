#include <donut/Color.hpp>
#include <donut/File.hpp>
#include <donut/Filesystem.hpp>
#include <donut/graphics/Error.hpp>
#include <donut/graphics/Font.hpp>
#include <donut/graphics/Renderer.hpp>
#include <donut/graphics/Texture.hpp>
#include <donut/math.hpp>

#include <algorithm>    // std::lower_bound
#include <cassert>      // assert
#include <cstddef>      // std::size_t, std::byte
#include <cstdint>      // std::uint32_t
#include <fmt/format.h> // fmt::format
#include <schrift.h>    // SFT..., sft_...
#include <utility>      // std::pair
#include <vector>       // std::vector

namespace donut::graphics {

Font::Font(const Filesystem& filesystem, const char* filepath, const FontOptions& options)
	: fontFileContents(filesystem.openFile(filepath).readAll())
	, font(sft_loadmem(fontFileContents.data(), fontFileContents.size()))
	, options(options) {
	if (!font) {
		throw Error{fmt::format("Failed to load font \"{}\".", filepath)};
	}
}

Font::Glyph Font::findGlyph(u32 characterSize, char32_t codePoint) const noexcept {
	const GlyphKey glyphKey{.characterSize = characterSize, .codePoint = codePoint};
	if (const auto it = std::lower_bound(sortedGlyphKeys.begin(), sortedGlyphKeys.end(), glyphKey); it != sortedGlyphKeys.end() && *it == glyphKey) {
		const std::size_t glyphIndex = static_cast<std::size_t>(it - sortedGlyphKeys.begin());
		assert(glyphIndex < glyphsSortedByKey.size());
		return glyphsSortedByKey[glyphIndex];
	}
	return {.positionInAtlas{}, .sizeInAtlas{}, .rendered = false};
}

std::pair<Font::Glyph, bool> Font::renderGlyph(Renderer& renderer, u32 characterSize, char32_t codePoint) {
	const GlyphKey glyphKey{.characterSize = characterSize, .codePoint = codePoint};
	const auto it = std::lower_bound(sortedGlyphKeys.begin(), sortedGlyphKeys.end(), glyphKey);
	const std::size_t glyphIndex = static_cast<std::size_t>(it - sortedGlyphKeys.begin());
	const bool foundGlyph = it != sortedGlyphKeys.end() && *it == glyphKey;
	if (foundGlyph) {
		assert(glyphIndex < glyphsSortedByKey.size());
		if (const Glyph glyph = glyphsSortedByKey[glyphIndex]; glyph.rendered) {
			return {glyph, false};
		}
	}

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

	const Glyph renderedGlyph{
		.positionInAtlas{static_cast<float>(x), static_cast<float>(y)},
		.sizeInAtlas{static_cast<float>(width), static_cast<float>(height)},
		.rendered = true,
	};

	if (foundGlyph) {
		assert(glyphIndex < sortedGlyphKeys.size());
		assert(glyphIndex < glyphsSortedByKey.size());
		glyphsSortedByKey[glyphIndex] = renderedGlyph;
	} else {
		assert(glyphIndex <= sortedGlyphKeys.size());
		assert(glyphIndex <= glyphsSortedByKey.size());
		sortedGlyphKeys.insert(sortedGlyphKeys.begin() + glyphIndex, glyphKey);
		try {
			glyphsSortedByKey.insert(glyphsSortedByKey.begin() + glyphIndex, renderedGlyph);
		} catch (...) {
			sortedGlyphKeys.erase(sortedGlyphKeys.begin() + glyphIndex);
			throw;
		}
	}
	return {renderedGlyph, true};
}

Font::GlyphMetrics Font::getGlyphMetrics(u32 characterSize, char32_t codePoint) const noexcept {
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
		return {.size{0.0f, 0.0f}, .bearing{0.0f, 0.0f}, .advance = 0.0f};
	}

	SFT_GMetrics gmetrics{};
	sft_gmetrics(&sft, glyph, &gmetrics);

	const std::size_t width = static_cast<std::size_t>(gmetrics.minWidth);
	const std::size_t height = static_cast<std::size_t>(gmetrics.minHeight);
	return {
		.size{static_cast<float>(width), static_cast<float>(height)},
		.bearing{static_cast<float>(gmetrics.leftSideBearing), static_cast<float>(gmetrics.yOffset)},
		.advance = static_cast<float>(gmetrics.advanceWidth),
	};
}

Font::LineMetrics Font::getLineMetrics(u32 characterSize) const noexcept {
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
		return {.ascender = 0.0f, .descender = 0.0f, .height = 0.0f};
	}

	return {
		.ascender = static_cast<float>(lmetrics.ascender),
		.descender = static_cast<float>(lmetrics.descender),
		.height = static_cast<float>(lmetrics.ascender - lmetrics.descender + lmetrics.lineGap),
	};
}

vec2 Font::getKerning(u32 characterSize, char32_t left, char32_t right) const noexcept {
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
		return {0.0f, 0.0f};
	}

	SFT_Glyph rightGlyph{};
	if (sft_lookup(&sft, SFT_UChar{right}, &rightGlyph) != 0) {
		return {0.0f, 0.0f};
	}

	SFT_Kerning kerning{};
	if (sft_kerning(&sft, leftGlyph, rightGlyph, &kerning) != 0) {
		return {0.0f, 0.0f};
	}

	return {static_cast<float>(kerning.xShift), static_cast<float>(kerning.yShift)};
}

void Font::markGlyphForRendering(u32 characterSize, char32_t codePoint) {
	const GlyphKey glyphKey{.characterSize = characterSize, .codePoint = codePoint};
	const auto it = std::lower_bound(sortedGlyphKeys.begin(), sortedGlyphKeys.end(), glyphKey);
	if (it != sortedGlyphKeys.end() && *it == glyphKey) {
		[[likely]];
		return;
	}
	const std::size_t glyphIndex = static_cast<std::size_t>(it - sortedGlyphKeys.begin());
	assert(glyphIndex <= sortedGlyphKeys.size());
	assert(glyphIndex <= glyphsSortedByKey.size());
	sortedGlyphKeys.insert(sortedGlyphKeys.begin() + glyphIndex, glyphKey);
	try {
		glyphKeysMarkedForRendering.push_back(glyphKey);
		try {
			glyphsSortedByKey.insert(glyphsSortedByKey.begin() + glyphIndex, Glyph{.positionInAtlas{}, .sizeInAtlas{}, .rendered = false});
		} catch (...) {
			glyphKeysMarkedForRendering.pop_back();
			throw;
		}
	} catch (...) {
		sortedGlyphKeys.erase(sortedGlyphKeys.begin() + glyphIndex);
		throw;
	}
}

bool Font::renderMarkedGlyphs(Renderer& renderer) {
	bool renderedAny = false;
	for (const GlyphKey glyphKey : glyphKeysMarkedForRendering) {
		if (renderGlyph(renderer, glyphKey.characterSize, glyphKey.codePoint).second) {
			renderedAny = true;
		}
	}
	glyphKeysMarkedForRendering.clear();
	return renderedAny;
}

bool Font::containsGlyphsMarkedForRendering() const noexcept {
	return !glyphKeysMarkedForRendering.empty();
}

void Font::prepareAtlasTexture(Renderer& renderer, bool resized) {
	if (atlasTexture) {
		if (resized) {
			atlasTexture.grow2D(renderer, atlasPacker.getResolution(), atlasPacker.getResolution(), Color::INVISIBLE);
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

void Font::FontDeleter::operator()(void* handle) const noexcept {
	sft_freefont(static_cast<SFT_Font*>(handle));
}

} // namespace donut::graphics
