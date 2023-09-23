#ifndef DONUT_GRAPHICS_FONT_HPP
#define DONUT_GRAPHICS_FONT_HPP

#include <donut/AtlasPacker.hpp>
#include <donut/Filesystem.hpp>
#include <donut/UniqueHandle.hpp>
#include <donut/graphics/Texture.hpp>
#include <donut/math.hpp>

#include <cstddef> // std::size_t, std::byte
#include <utility> // std::pair
#include <vector>  // std::vector

namespace donut::graphics {

class Renderer; // Forward declaration, to avoid including Renderer.hpp.

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
 * Typeface describing an assortment of character glyphs that may be rendered
 * on-demand into an expanding texture atlas, for use in Text rendering.
 */
class Font {
public:
	/**
	 * Information about a single glyph's entry in the texture atlas.
	 */
	struct Glyph {
		vec2 positionInAtlas; ///< Position of this glyph's rectangle in the texture atlas, in texels. Invalid if Glyph::rendered is false.
		vec2 sizeInAtlas;     ///< Size of this glyph's rectangle in the texture atlas, in texels. Invalid if Glyph::rendered is false.
		bool rendered;        ///< True if the glyph has been rendered and has a valid rectangle in the texture atlas, false otherwise.
	};

	/**
	 * Dimensions of a single glyph in this font, for shaping text.
	 */
	struct GlyphMetrics {
		vec2 size;     ///< Size of this glyph's rectangle when rendered, in pixels.
		vec2 bearing;  ///< Offset from the baseline to apply to the glyph's rectangle position when rendering this glyph.
		float advance; ///< Horizontal offset to apply in order to advance to the next glyph position, excluding any kerning.
	};

	/**
	 * Vertical dimensions for shaping lines of text with this font.
	 */
	struct LineMetrics {
		float ascender;  ///< Vertical offset from the baseline to the visual top of the text.
		float descender; ///< Vertical offset from the baseline to the visual bottom of the text.
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
	 * Look up the information about a glyph's entry in the texture atlas for a
	 * specific code point.
	 *
	 * \param characterSize character size of the glyph to search for.
	 * \param codePoint Unicode code point of the glyph to search for.
	 *
	 * \return the glyph information, see Glyph.
	 *
	 * \sa renderGlyph()
	 * \sa getAtlasTexture()
	 */
	[[nodiscard]] Glyph findGlyph(u32 characterSize, char32_t codePoint) const noexcept;

	/**
	 * Render the glyph for a specific character and store it in the texture
	 * atlas, if it has not already been rendered.
	 *
	 * \param renderer renderer to use for rendering the glyph.
	 * \param characterSize character size to render the glyph at.
	 * \param codePoint Unicode code point of the glyph to render.
	 *
	 * \return a pair where:
	 *         - the first element contains information about the rendered
	 *           glyph, and
	 *         - the second element contains a bool that is true if the glyph
	 *           was actually rendered, or false if the glyph had already been
	 *           rendered previously.
	 *
	 * \throws graphics::Error on failure to render the glyph.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note If the specified glyph has already been rendered previously, no
	 *       modification is made, and the already rendered glyph is returned.
	 *       In this case, the function is guaranteed to not throw any
	 *       exceptions.
	 *
	 * \sa findGlyph()
	 * \sa getGlyphMetrics()
	 * \sa getAtlasTexture()
	 */
	std::pair<Glyph, bool> renderGlyph(Renderer& renderer, u32 characterSize, char32_t codePoint);

	/**
	 * Get the dimensions of a single glyph in this font, for shaping text.
	 *
	 * \param characterSize character size to get the glyph metrics of.
	 * \param codePoint Unicode code point to get the glyph metrics of.
	 *
	 * \return the glyph metrics of the given code point at the given character
	 *         size, see GlyphMetrics.
	 *
	 * \sa findGlyph()
	 * \sa renderGlyph()
	 */
	[[nodiscard]] GlyphMetrics getGlyphMetrics(u32 characterSize, char32_t codePoint) const noexcept;

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
	 * Enqueue a glyph for rendering on the next call to renderMarkedGlyphs() if
	 * it has not already been rendered.
	 *
	 * \param characterSize character size to render the glyph at.
	 * \param codePoint Unicode code point of the glyph to render.
	 *
	 * \throws graphics::Error on failure to mark the glyph.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \sa renderMarkedGlyphs()
	 */
	void markGlyphForRendering(u32 characterSize, char32_t codePoint);

	/**
	 * Render all glyphs marked using markGlyphForRendering() that have not
	 * already been rendered.
	 *
	 * \param renderer renderer to use for rendering the glyphs.
	 *
	 * \return true if at least one marked glyph needed to be rendered, false if
	 *         no rendering took place because all marked glyphs were already
	 *         rendered.
	 *
	 * \throws graphics::Error on failure to render a glyph.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \sa markGlyphForRendering()
	 * \sa containsGlyphsMarkedForRendering()
	 */
	bool renderMarkedGlyphs(Renderer& renderer);

	/**
	 * Check if any unrendered glyphs have been marked for rendering.
	 *
	 * \return true if some marked glyph might need to be rendered, false if
	 *         certainly no unrendered glyphs have been marked.
	 */
	[[nodiscard]] bool containsGlyphsMarkedForRendering() const noexcept;

	/**
	 * Get the texture atlas to use when rendering glyphs from this font.
	 *
	 * \return a read-only reference to a square texture containing all loaded
	 *         glyphs.
	 *
	 * \sa findGlyph()
	 * \sa loadGlyph()
	 */
	[[nodiscard]] const Texture& getAtlasTexture() const noexcept {
		return atlasTexture;
	}

private:
	struct FontDeleter {
		void operator()(void* handle) const noexcept;
	};

	struct GlyphKey {
		u32 characterSize;
		char32_t codePoint;

		[[nodiscard]] constexpr auto operator<=>(const GlyphKey&) const = default;
	};

	static constexpr std::size_t INITIAL_RESOLUTION = 128;
	static constexpr std::size_t PADDING = 6;

	void prepareAtlasTexture(Renderer& renderer, bool resized);

	std::vector<std::byte> fontFileContents;
	UniqueHandle<void*, FontDeleter> font;
	AtlasPacker<INITIAL_RESOLUTION, PADDING> atlasPacker{};
	Texture atlasTexture{};
	std::vector<GlyphKey> sortedGlyphKeys{};
	std::vector<Glyph> glyphsSortedByKey{};
	std::vector<GlyphKey> glyphKeysMarkedForRendering{};
	FontOptions options;
};

} // namespace donut::graphics

#endif
