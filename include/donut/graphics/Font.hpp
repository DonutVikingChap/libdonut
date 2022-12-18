#ifndef DONUT_GRAPHICS_FONT_HPP
#define DONUT_GRAPHICS_FONT_HPP

#include <donut/AtlasPacker.hpp>
#include <donut/graphics/Texture.hpp>

#include <cstddef>       // std::size_t, std::byte
#include <cstdint>       // std::uint32_t, std::uint64_t
#include <glm/glm.hpp>   // glm::...
#include <string_view>   // std::string_view
#include <unordered_map> // std::unordered_map
#include <vector>        // std::vector

namespace donut {
namespace graphics {

class Renderer;

struct FontOptions {
	bool useLinearFiltering = false;
};

class Font {
public:
	struct Glyph {
		glm::vec2 textureOffset;
		glm::vec2 textureScale;
		glm::vec2 position;
		glm::vec2 size;
		glm::vec2 bearing;
		float advance;
	};

	struct ShapedText {
		struct ShapedGlyph {
			glm::vec2 offset;
			glm::vec2 size;
			glm::vec2 textureOffset;
			glm::vec2 textureScale;
		};

		std::vector<ShapedGlyph> shapedGlyphs;
		glm::vec2 extentsMin;
		glm::vec2 extentsMax;
		std::size_t rowCount;
	};

	struct LineMetrics {
		float ascender;
		float descender;
		float height;
	};

	explicit Font(const char* filepath, const FontOptions& options = {});
	~Font() = default;

	Font(const Font&) = delete;
	Font(Font&& other) noexcept = default;
	Font& operator=(const Font& other) = delete;
	Font& operator=(Font&& other) noexcept = default;

	explicit operator bool() const noexcept {
		return static_cast<bool>(font);
	}

	[[nodiscard]] const Glyph* findGlyph(std::uint32_t characterSize, char32_t codePoint) const noexcept;
	const Glyph& loadGlyph(Renderer& renderer, std::uint32_t characterSize, char32_t codePoint);
	[[nodiscard]] ShapedText shapeText(Renderer& renderer, std::uint32_t characterSize, std::u8string_view string, glm::vec2 scale = {1.0f, 1.0f});
	[[nodiscard]] ShapedText shapeText(Renderer& renderer, std::uint32_t characterSize, std::string_view string, glm::vec2 scale = {1.0f, 1.0f});

	[[nodiscard]] LineMetrics getLineMetrics(std::uint32_t characterSize) const noexcept;
	[[nodiscard]] glm::vec2 getKerning(std::uint32_t characterSize, char32_t left, char32_t right) const noexcept;

	[[nodiscard]] const Texture& getAtlasTexture() const noexcept {
		return atlasTexture;
	}

private:
	struct FontDeleter {
		void operator()(void* handle) const noexcept;
	};

	using GlyphKey = std::uint64_t;

	static constexpr std::size_t INITIAL_RESOLUTION = 128;
	static constexpr std::size_t PADDING = 6;

	[[nodiscard]] static constexpr GlyphKey makeGlyphKey(std::uint32_t characterSize, char32_t codePoint) noexcept {
		static_assert(sizeof(GlyphKey) >= sizeof(characterSize) + sizeof(codePoint));
		static_assert(sizeof(codePoint) == 4);
		return static_cast<GlyphKey>(characterSize) << 32 | static_cast<GlyphKey>(codePoint);
	}

	void prepareAtlasTexture(Renderer& renderer, bool resized);

	[[nodiscard]] Glyph renderGlyph(Renderer& renderer, std::uint32_t characterSize, char32_t codePoint);

	std::vector<std::byte> fontFileContents;
	Resource<void*, FontDeleter, nullptr> font;
	AtlasPacker<INITIAL_RESOLUTION, PADDING> atlasPacker{};
	Texture atlasTexture{};
	std::unordered_map<GlyphKey, Glyph> glyphs{};
	FontOptions options;
};

} // namespace graphics
} // namespace donut

#endif
