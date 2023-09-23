#ifndef DONUT_GRAPHICS_TEXT_HPP
#define DONUT_GRAPHICS_TEXT_HPP

#include <donut/math.hpp>

#include <cstddef>     // std::size_t
#include <limits>      // std::numeric_limits
#include <span>        // std::span
#include <string_view> // std::string_view, std::u8string_view
#include <vector>      // std::vector

namespace donut::graphics {

class Font; // Forward declaration, to avoid including Font.hpp.

/**
 * Facility for shaping text, according to a Font, into renderable glyphs.
 */
class Text {
public:
	/**
	 * Data required to render a single shaped glyph relative to at any given
	 * starting position.
	 *
	 * \sa ShapedGlyphInfo
	 * \sa ShapedLineInfo
	 */
	struct ShapedGlyph {
		Font* font;         ///< Non-owning read-only non-null pointer to the font used to shape this glyph.
		vec2 shapedOffset;  ///< Scaled offset from the starting position to draw this glyph at, in pixels.
		vec2 shapedSize;    ///< Scaled size of this glyph's rectangle, in pixels.
		u32 characterSize;  ///< Character size that this glyph was shaped at.
		char32_t codePoint; ///< Unicode code point of this glyph.
	};

	/**
	 * Additional information about a single shaped glyph, including some data
	 * that is not strictly required for simple rendering.
	 *
	 * \sa ShapedGlyph
	 * \sa ShapedLineInfo
	 */
	struct ShapedGlyphInfo {
		vec2 shapedOffset;           ///< Scaled offset from the starting position to draw this glyph at, in pixels.
		vec2 shapedAdvance;          ///< Scaled offset to apply in order to advance to the next glyph position, including kerning.
		std::size_t shapedLineIndex; ///< Index of the ShapedLineInfo corresponding to the line that this glyph is part of.
		std::size_t stringOffset;    ///< Byte offset in the input string of the first code unit that this glyph originated from.
	};

	/**
	 * Information about a line of shaped glyphs, including some data that is
	 * not strictly required for simple rendering.
	 *
	 * \sa ShapedGlyph
	 * \sa ShapedGlyphInfo
	 */
	struct ShapedLineInfo {
		vec2 shapedOffset;             ///< Scaled offset of the baseline at the start of this line of text.
		vec2 shapedSize;               ///< Scaled total size of this line.
		std::size_t shapedGlyphOffset; ///< Index of the ShapedGlyph and ShapedGlyphInfo corresponding to the first glyph that is part of this line.
		std::size_t stringOffset;      ///< Byte offset in the input string of the first code unit that the first glyph that is part of this line originated from.
	};

	/**
	 * Result of the shape() function.
	 */
	struct ShapeResult {
		/**
		 * Index, into the lists returned by getShapedGlyphs() and
		 * getShapedGlyphsInfo(), of the ShapedGlyph and ShapedGlyphInfo
		 * corresponding to the first glyph that was shaped.
		 *
		 * If no glyphs were shaped, this is the index that the first glyph
		 * would have had if it was shaped, i.e. the previous size of the lists.
		 */
		std::size_t shapedGlyphOffset;

		/**
		 * Index, into the list returned by getShapedLinesInfo(), of the
		 * ShapedLineInfo corresponding to the first line that was shaped.
		 */
		std::size_t shapedLineOffset;
	};

	/**
	 * Construct an empty text.
	 */
	Text() noexcept = default;

	/**
	 * Construct a shaped text from a UTF-8 string.
	 *
	 * \param font font to shape the glyphs with.
	 * \param characterSize character size to shape the glyphs at.
	 * \param string UTF-8 encoded text string to shape.
	 * \param offset relative offset from the starting position to begin shaping
	 *        at.
	 * \param scale scaling to apply to the size of the shaped glyphs. The
	 *        result is affected by FontOptions::useLinearFiltering.
	 *
	 * \throws graphics::Error on failure to shape a glyph.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note Right-to-left text shaping is currently not supported.
	 * \note Grapheme clusters are currently not supported, and may be shaped
	 *       incorrectly. Only one Unicode code point is shaped at a time.
	 *
	 * \warning If the string contains invalid UTF-8, the invalid code points
	 *          will generate unspecified glyphs that may have any appearance.
	 *
	 * \remark The best visual results are usually achieved when the text is
	 *         shaped at an appropriate character size to begin with, rather
	 *         than relying on the scaling of this function. As such, the scale
	 *         parameter should generally be kept at (1, 1) unless many
	 *         different character sizes are used with this font and there is a
	 *         strict requirement on the maximum size of the texture atlas.
	 *
	 * \sa shape()
	 */
	Text(Font& font, u32 characterSize, std::u8string_view string, vec2 offset = {0.0f, 0.0f}, vec2 scale = {1.0f, 1.0f}) {
		shape(font, characterSize, string, offset, scale);
	}

	/**
	 * Helper overload of Text() that takes an arbitrary byte string and
	 * interprets it as UTF-8.
	 *
	 * \sa Text(const Font&, u32, std::u8string_view, vec2, vec2)
	 */
	Text(Font& font, u32 characterSize, std::string_view string, vec2 offset = {0.0f, 0.0f}, vec2 scale = {1.0f, 1.0f}) {
		shape(font, characterSize, string, offset, scale);
	}

	/**
	 * Erase all shaped glyphs and reset the text to an empty state.
	 */
	void clear() noexcept {
		shapedGlyphs.clear();
		shapedGlyphsInfo.clear();
		shapedLinesInfo.clear();
		minExtent = {std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()};
		maxExtent = {-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()};
	}

	/**
	 * Use a font to shape a string of UTF-8 encoded text into a sequence of
	 * glyphs that are ready to be drawn at a given offset, relative to any
	 * starting position.
	 *
	 * \param font font to shape the glyphs with.
	 * \param characterSize character size to shape the glyphs at.
	 * \param string UTF-8 encoded text string to shape.
	 * \param offset relative offset from the starting position to begin shaping
	 *        at.
	 * \param scale scaling to apply to the size of the shaped glyphs. The
	 *        result is affected by FontOptions::useLinearFiltering.
	 *
	 * \return see ShapeResult.
	 *
	 * \throws graphics::Error on failure to shape a glyph.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note Right-to-left text shaping is currently not supported.
	 * \note Grapheme clusters are currently not supported, and may be shaped
	 *       incorrectly. Only one Unicode code point is shaped at a time.
	 *
	 * \warning If the string contains invalid UTF-8, the invalid code points
	 *          will generate unspecified glyphs that may have any appearance.
	 *
	 * \remark The best visual results are usually achieved when the text is
	 *         shaped at an appropriate character size to begin with, rather
	 *         than relying on the scaling of this function. As such, the scale
	 *         parameter should generally be kept at (1, 1) unless many
	 *         different character sizes are used with this font and there is a
	 *         strict requirement on the maximum size of the texture atlas.
	 *
	 * \sa getShapedGlyphs()
	 * \sa getShapedGlyphsInfo()
	 * \sa getShapedLinesInfo()
	 * \sa getMinExtent()
	 * \sa getMaxExtent()
	 */
	ShapeResult shape(Font& font, u32 characterSize, std::u8string_view string, vec2 offset = {0.0f, 0.0f}, vec2 scale = {1.0f, 1.0f});

	/**
	 * Helper overload of shape() that takes an arbitrary byte string and
	 * interprets it as UTF-8.
	 *
	 * \sa shape(Font&, u32, std::u8string_view, vec2, vec2)
	 */
	ShapeResult shape(Font& font, u32 characterSize, std::string_view string, vec2 offset = {0.0f, 0.0f}, vec2 scale = {1.0f, 1.0f}) {
		static_assert(sizeof(char) == sizeof(char8_t));
		static_assert(alignof(char) == alignof(char8_t));
		return shape(font, characterSize, std::u8string_view{reinterpret_cast<const char8_t*>(string.data()), string.size()}, offset, scale);
	}

	/**
	 * Helper function that is equivalent to clear() followed by shape().
	 *
	 * \sa clear()
	 * \sa shape()
	 */
	ShapeResult reshape(Font& font, u32 characterSize, std::string_view string, vec2 offset = {0.0f, 0.0f}, vec2 scale = {1.0f, 1.0f}) {
		clear();
		return shape(font, characterSize, string, offset, scale);
	}

	/**
	 * Helper function that is equivalent to clear() followed by shape().
	 *
	 * \sa clear()
	 * \sa shape()
	 */
	ShapeResult reshape(Font& font, u32 characterSize, std::u8string_view string, vec2 offset = {0.0f, 0.0f}, vec2 scale = {1.0f, 1.0f}) {
		clear();
		return shape(font, characterSize, string, offset, scale);
	}

	/**
	 * Get the list of ShapedGlyph data for all shaped glyphs.
	 *
	 * \return a non-owning read-only random-access view over the ShapedGlyph
	 *         data.
	 *
	 * \sa shape()
	 * \sa getShapedGlyphsInfo()
	 * \sa getShapedLinesInfo()
	 */
	[[nodiscard]] std::span<const ShapedGlyph> getShapedGlyphs() const noexcept {
		return shapedGlyphs;
	}

	/**
	 * Get the list of ShapedGlyphInfo data for all shaped glyphs.
	 *
	 * \return a non-owning read-only random-access view over the
	 *         ShapedGlyphInfo data.
	 *
	 * \sa shape()
	 * \sa getShapedGlyphs()
	 * \sa getShapedLinesInfo()
	 */
	[[nodiscard]] std::span<const ShapedGlyphInfo> getShapedGlyphsInfo() const noexcept {
		return shapedGlyphsInfo;
	}

	/**
	 * Get the list of ShapedLineInfo data for all shaped lines.
	 *
	 * \return a non-owning read-only random-access view over the ShapedLineInfo
	 *         data.
	 *
	 * \sa shape()
	 * \sa getShapedGlyphs()
	 * \sa getShapedGlyphsInfo()
	 */
	[[nodiscard]] std::span<const ShapedLineInfo> getShapedLinesInfo() const noexcept {
		return shapedLinesInfo;
	}

	/**
	 * Get the minimum extent of the shaped text.
	 *
	 * \return the offset of the bottom left corner of the smallest rectangular
	 *         area that spans all glyph rectangles of this text.
	 */
	[[nodiscard]] vec2 getMinExtent() const noexcept {
		return minExtent;
	}

	/**
	 * Get the maximum extent of the shaped text.
	 *
	 * \return the offset of the top right corner of the smallest rectangular
	 *         area that spans all glyph rectangles of this text.
	 */
	[[nodiscard]] vec2 getMaxExtent() const noexcept {
		return maxExtent;
	}

private:
	std::vector<ShapedGlyph> shapedGlyphs{};
	std::vector<ShapedGlyphInfo> shapedGlyphsInfo{};
	std::vector<ShapedLineInfo> shapedLinesInfo{};
	vec2 minExtent{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()};
	vec2 maxExtent{-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()};
};

} // namespace donut::graphics

#endif
