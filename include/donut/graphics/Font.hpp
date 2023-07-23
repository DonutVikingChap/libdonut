#ifndef DONUT_GRAPHICS_FONT_HPP
#define DONUT_GRAPHICS_FONT_HPP

#include <donut/AtlasPacker.hpp>
#include <donut/Filesystem.hpp>
#include <donut/UniqueHandle.hpp>
#include <donut/graphics/Texture.hpp>
#include <donut/math.hpp>

#include <cstddef>       // std::size_t, std::byte
#include <string_view>   // std::string_view, std::u8string_view
#include <unordered_map> // std::unordered_map
#include <vector>        // std::vector

namespace donut::graphics {

class Renderer; // Forward declaration, to avoid a circular include of Renderer.hpp.

/**
 * Configuration options for a Font.
 */
struct FontOptions {
	/**
	 * Use bilinear filtering rather than nearest-neighbor interpolation when
	 * rendering text at a non-1:1 scale using this font.
	 *
	 * When set to true, this will cause scaled text to appear smoother compared
	 * to regular blocky nearest-neighbor scaling. Using linear filtering can
	 * help reduce aliasing artifacts on the glyph edges, but also makes the
	 * text more blurry.
	 *
	 * \note Regardless of this option, The best results are usually achieved
	 *       when text is rendered at an appropriate character size to begin
	 *       with, rather than relying on scaling.
	 */
	bool useLinearFiltering = false;
};

/**
 * Facility for shaping text, according to a loaded font file, into renderable
 * glyphs stored in an expanding texture atlas.
 */
class Font {
public:
	/**
	 * Information about a single glyph stored in the texture atlas.
	 */
	struct Glyph {
		vec2 textureOffset; ///< Texture coordinate offset of this glyph's rectangle in the texture atlas.
		vec2 textureScale;  ///< Texture coordinate scale of this glyph's rectangle in the texture atlas.
		vec2 position;      ///< Position of this glyph's rectangle in the texture atlas, in texels.
		vec2 size;          ///< Size of this glyph's rectangle in the texture atlas, in texels.
		vec2 bearing;       ///< Baseline offset to apply to the glyph's rectangle position when rendering this glyph.
		float advance;      ///< Horizontal offset to apply in order to advance to the next glyph position, excluding any kerning.
	};

	/**
	 * Information about a string of shaped glyphs that has been prepared for
	 * rendering at any given position.
	 */
	struct ShapedText {
		/**
		 * Information about a single shaped glyph that has been prepered for
		 * rendering at any given position.
		 */
		struct ShapedGlyph {
			vec2 offset;        ///< Scaled offset from the starting position to draw this glyph at, in pixels.
			vec2 size;          ///< Scaled size of this glyph's rectangle, in pixels.
			vec2 textureOffset; ///< Texture coordinate offset of this glyph's rectangle in the texture atlas.
			vec2 textureScale;  ///< Texture coordinate scale of this glyph's rectangle in the texture atlas.
		};

		std::vector<ShapedGlyph> shapedGlyphs; ///< Sequence of shaped glyphs making up this shaped string.
		vec2 extentsMin;                       ///< Offset of the top left corner of the smallest rectangular area that spans all glyph rectangles of this string.
		vec2 extentsMax;                       ///< Offset of the bottom right corner of the smallest rectangular area that spans all glyph rectangles of this string.
		std::size_t rowCount;                  ///< Total number of lines of text in the shaped string.
	};

	/**
	 * Vertical dimensions for shaping lines of text with this font.
	 */
	struct LineMetrics {
		float ascender;  ///< Distance from the baseline to the visual top of the text.
		float descender; ///< Distance from the baseline to the visual bottom of the text.
		float height;    ///< Vertical offset to apply in order to advance to the next line.
	};

	/**
	 * Load a font from a virtual file.
	 *
	 * The supported file formats are:
	 * - TrueType (.ttf)
	 * - OpenType (.otf)
	 *
	 * \param filesystem virtual filesystem to load the file from.
	 * \param filepath virtual filepath of the font file to load.
	 * \param options font options, see FontOptions.
	 *
	 * \throws File::Error on failure to open the file.
	 * \throws graphics::Error on failure to load a font from the file.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note The only supported text encoding is Unicode.
	 * \note Only TrueType fonts are fully supported. OpenType extensions that
	 *       are not a part of TrueType may not work.
	 */
	explicit Font(const Filesystem& filesystem, const char* filepath, const FontOptions& options = {});

	/** Destructor. */
	~Font() = default;

	/**
	 * Copying a font is not allowed, since the memory location of the file
	 * contents must remain stable.
	 */
	Font(const Font&) = delete;

	/** Move constructor. */
	Font(Font&& other) noexcept = default;

	/** 
	 * Copying a font is not allowed, since the memory location of the file
	 * contents must remain stable.
	 */
	Font& operator=(const Font& other) = delete;

	/** Move assignment. */
	Font& operator=(Font&& other) noexcept = default;

	/**
	 * Look up the glyph information stored in the texture atlas for a specific
	 * character.
	 *
	 * \param characterSize character size to search for.
	 * \param codePoint Unicode code point to search for.
	 *
	 * \return if found, returns a read-only non-owning pointer to the stored
	 *         glyph information. Otherwise, returns nullptr.
	 *
	 * \note The glyph will not be found unless it has previously been loaded
	 *       through a call to loadGlyph() or shapeText().
	 *
	 * \warning The returned pointer will be invalidated on the next call to
	 *          loadGlyph() or shapeText() that loads a new glyph. Therefore,
	 *          the glyph information should be copied if it needs to be stored
	 *          for any longer period of time.
	 *
	 * \sa loadGlyph()
	 * \sa shapeText()
	 * \sa getAtlasTexture()
	 */
	[[nodiscard]] const Glyph* findGlyph(u32 characterSize, char32_t codePoint) const noexcept;

	/**
	 * Load the glyph information for a specific character and store it in the
	 * texture atlas, if it has not already been loaded.
	 *
	 * \param renderer renderer to use for rendering the glyph into the texture
	 *        atlas.
	 * \param characterSize character size to render the glyph at.
	 * \param codePoint Unicode code point to load.
	 *
	 * \return a reference to the loaded glyph information.
	 *
	 * \throws graphics::Error on failure to render the glyph.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note If the specified glyph has already been loaded previously, no
	 *       modification is made, and a reference to the already loaded glyph
	 *       is returned. In this case, the function is guaranteed to not throw
	 *       any exceptions.
	 *
	 * \warning The returned reference will be invalidated on the next call to
	 *          loadGlyph() or shapeText() that loads a new glyph. Therefore,
	 *          the glyph information should be copied if it needs to be stored
	 *          for any longer period of time.
	 *
	 * \sa findGlyph()
	 * \sa shapeText()
	 * \sa getAtlasTexture()
	 */
	const Glyph& loadGlyph(Renderer& renderer, u32 characterSize, char32_t codePoint);

	/**
	 * Use the font to shape a UTF-8 encoded text string into a sequence of
	 * glyphs that are ready to be drawn relative to any starting position.
	 *
	 * \param renderer renderer to use for rendering any glyphs that were not
	 *        already loaded into the texture atlas.
	 * \param characterSize character size to render the glyphs at.
	 * \param string UTF-8 encoded text string to shape.
	 * \param scale scaling to apply to the size of the rendered glyphs. The
	 *        result is affected by FontOptions::useLinearFiltering.
	 *
	 * \return information about the sequence of shaped glyphs, see ShapedText.
	 *
	 * \throws graphics::Error on failure to render a glyph.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note The best visual results are usually achieved when the text is
	 *       rendered at an appropriate character size to begin with, rather
	 *       than relying on scaling. As such, the scale parameter should
	 *       generally be kept at (1, 1) unless many different character sizes
	 *       are used with this font and there is a strict requirement on the
	 *       maximum size of the texture atlas.
	 * \note Right-to-left text shaping is currently not supported.
	 * \note Grapheme clusters are currently not supported, and may be rendered
	 *       incorrectly. Only one Unicode code point is rendered at a time.
	 *
	 * \warning If the string contains invalid UTF-8, the invalid code points
	 *          will generate unspecified glyphs that may have any appearance.
	 *
	 * \sa findGlyph()
	 * \sa loadGlyph()
	 * \sa getAtlasTexture()
	 */
	[[nodiscard]] ShapedText shapeText(Renderer& renderer, u32 characterSize, std::u8string_view string, vec2 scale = {1.0f, 1.0f});

	/**
	 * Helper overload of shapeText() that takes an arbitrary byte string and
	 * interprets it as UTF-8.
	 *
	 * \sa shapeText(Renderer&, u32, std::u8string_view, vec2)
	 */
	[[nodiscard]] ShapedText shapeText(Renderer& renderer, u32 characterSize, std::string_view string, vec2 scale = {1.0f, 1.0f});

	/**
	 * Get the vertical dimensions for shaping lines of text with this font.
	 *
	 * \param characterSize character size to get the line metrics of.
	 *
	 * \return the line metrics at the given character size, see LineMetrics.
	 */
	[[nodiscard]] LineMetrics getLineMetrics(u32 characterSize) const noexcept;

	/**
	 * Get the kerning offset to use between a pair of adjacent character glyphs
	 * while shaping text.
	 *
	 * \param characterSize character size of the glyphs to get the kerning of.
	 * \param left Unicode code point of the left glyph in the adjacent pair.
	 * \param right Unicode code point of the right glyph in the adjacent pair.
	 *
	 * \return if the font contains a valid glyph for both the left and the
	 *         right characters, returns the additional offset to advance the
	 *         position by when going from the left glyph to the right glyph.
	 *         Otherwise, returns (0, 0).
	 */
	[[nodiscard]] vec2 getKerning(u32 characterSize, char32_t left, char32_t right) const noexcept;

	/**
	 * Get the texture atlas to use when rendering glyphs from this font.
	 *
	 * \return a read-only reference to a square texture containing all loaded
	 *         glyphs.
	 *
	 * \sa findGlyph()
	 * \sa loadGlyph()
	 * \sa shapeText()
	 */
	[[nodiscard]] const Texture& getAtlasTexture() const noexcept {
		return atlasTexture;
	}

private:
	struct FontDeleter {
		void operator()(void* handle) const noexcept;
	};

	using GlyphKey = u64;

	static constexpr std::size_t INITIAL_RESOLUTION = 128;
	static constexpr std::size_t PADDING = 6;

	[[nodiscard]] static constexpr GlyphKey makeGlyphKey(u32 characterSize, char32_t codePoint) noexcept {
		static_assert(sizeof(GlyphKey) >= sizeof(characterSize) + sizeof(codePoint));
		static_assert(sizeof(codePoint) == 4);
		return static_cast<GlyphKey>(characterSize) << 32 | static_cast<GlyphKey>(codePoint);
	}

	void prepareAtlasTexture(Renderer& renderer, bool resized);

	[[nodiscard]] Glyph renderGlyph(Renderer& renderer, u32 characterSize, char32_t codePoint);

	std::vector<std::byte> fontFileContents;
	UniqueHandle<void*, FontDeleter> font;
	AtlasPacker<INITIAL_RESOLUTION, PADDING> atlasPacker{};
	Texture atlasTexture{};
	std::unordered_map<GlyphKey, Glyph> glyphs{};
	FontOptions options;
};

} // namespace donut::graphics

#endif
