#include <donut/graphics/Font.hpp>
#include <donut/graphics/Text.hpp>
#include <donut/math.hpp>
#include <donut/unicode.hpp>

#include <cstddef>     // std::size_t
#include <string_view> // std::u8string_view

namespace donut::graphics {

Text::ShapeResult Text::shape(Font& font, u32 characterSize, std::u8string_view string, vec2 offset, vec2 scale) {
	const std::size_t baseShapedGlyphOffset = shapedGlyphsInfo.size();
	const std::size_t baseShapedLineOffset = shapedLinesInfo.size();
	const vec2 previousMinExtent = minExtent;
	const vec2 previousMaxExtent = maxExtent;
	try {
		const vec2 baseOffset = offset;
		const Font::LineMetrics lineMetrics = font.getLineMetrics(characterSize);
		const float lineAscender = lineMetrics.ascender * scale.y;
		const float lineDescender = lineMetrics.descender * scale.y;
		const float lineHeight = round(lineMetrics.height * scale.y);

		minExtent = min(minExtent, offset);
		maxExtent = max(maxExtent, vec2{offset.x, offset.y + lineAscender});

		shapedLinesInfo.push_back(Text::ShapedLineInfo{
			.shapedOffset = offset,
			.shapedSize{0.0f, lineHeight},
			.shapedGlyphOffset = 0,
			.stringOffset = 0,
		});

		const unicode::UTF8View codePoints{string};
		for (auto it = codePoints.begin(); it != codePoints.end();) {
			const std::size_t stringOffset = static_cast<std::size_t>(it.base() - codePoints.begin().base());
			if (const char32_t codePoint = *it++; codePoint == '\n') {
				shapedLinesInfo.back().shapedSize.x = floor(offset.x - baseOffset.x);
				maxExtent.x = max(maxExtent.x, offset.x);
				offset.x = baseOffset.x;
				offset.y -= lineHeight;
				shapedLinesInfo.push_back(Text::ShapedLineInfo{
					.shapedOffset = offset,
					.shapedSize{0.0f, lineHeight},
					.shapedGlyphOffset = shapedGlyphsInfo.size(),
					.stringOffset = static_cast<std::size_t>(it.base() - codePoints.begin().base()),
				});
			} else {
				const Font::GlyphMetrics& glyphMetrics = font.getGlyphMetrics(characterSize, codePoint);
				const vec2 kerning = font.getKerning(characterSize, codePoint, (it == codePoints.end()) ? char32_t{0} : *it);
				const vec2 shapedOffset = floor(offset + glyphMetrics.bearing * scale);
				const vec2 shapedSize = glyphMetrics.size * scale;
				const vec2 shapedAdvance = vec2{glyphMetrics.advance + kerning.x, kerning.y} * scale;
				shapedGlyphs.push_back(Text::ShapedGlyph{
					.font = &font,
					.shapedOffset = shapedOffset,
					.shapedSize = shapedSize,
					.characterSize = characterSize,
					.codePoint = codePoint,
				});
				shapedGlyphsInfo.push_back(Text::ShapedGlyphInfo{
					.shapedOffset = shapedOffset,
					.shapedAdvance = shapedAdvance,
					.shapedLineIndex = shapedLinesInfo.size() - 1,
					.stringOffset = stringOffset,
				});
				offset += shapedAdvance;
			}
		}

		shapedLinesInfo.back().shapedSize.x = floor(offset.x - baseOffset.x);
		maxExtent.x = max(maxExtent.x, offset.x);
		minExtent.y = min(minExtent.y, offset.y + lineDescender);
	} catch (...) {
		shapedGlyphs.resize(baseShapedGlyphOffset);
		shapedGlyphsInfo.resize(baseShapedGlyphOffset);
		shapedLinesInfo.resize(baseShapedLineOffset);
		minExtent = previousMinExtent;
		maxExtent = previousMaxExtent;
		throw;
	}
	return {
		.shapedGlyphOffset = baseShapedGlyphOffset,
		.shapedLineOffset = baseShapedLineOffset,
	};
}

} // namespace donut::graphics
