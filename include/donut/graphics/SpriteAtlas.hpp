#ifndef DONUT_GRAPHICS_SPRITE_ATLAS_HPP
#define DONUT_GRAPHICS_SPRITE_ATLAS_HPP

#include <donut/Color.hpp>
#include <donut/graphics/AtlasPacker.hpp>
#include <donut/graphics/ImageLDR.hpp>
#include <donut/graphics/Texture.hpp>

#include <cstddef>     // std::size_t
#include <glm/glm.hpp> // glm::...
#include <vector>      // std::vector

namespace donut {
namespace graphics {

class Renderer;

class SpriteAtlas {
public:
	struct SpriteId {
	private:
		friend SpriteAtlas;

		constexpr explicit SpriteId(std::size_t index) noexcept
			: index(index) {}

		std::size_t index;
	};

	struct Sprite {
		glm::vec2 textureOffset{};
		glm::vec2 textureScale{};
		glm::vec2 position{};
		glm::vec2 size{};
	};

	[[nodiscard]] SpriteId insert(Renderer& renderer, ImageLDRView image) {
		const auto [x, y, resized] = atlasPacker.insertRectangle(image.getWidth(), image.getHeight());
		prepareAtlasTexture(renderer, resized);

		atlasTexture.pasteImage2D(image, x, y);

		const glm::vec2 textureSize = atlasTexture.getSize();
		const glm::vec2 position{static_cast<float>(x), static_cast<float>(y)};
		const glm::vec2 size{static_cast<float>(image.getWidth()), static_cast<float>(image.getHeight())};
		const std::size_t index = sprites.size();
		sprites.push_back({
			.textureOffset = position / textureSize,
			.textureScale = size / textureSize,
			.position = position,
			.size = size,
		});
		return SpriteId{index};
	}

	[[nodiscard]] const Sprite& getSprite(SpriteId id) const noexcept {
		return sprites[id.index];
	}

	[[nodiscard]] const Texture& getAtlasTexture() const noexcept {
		return atlasTexture;
	}

private:
	void prepareAtlasTexture(Renderer& renderer, bool resized) {
		if (atlasTexture) {
			if (resized) {
				atlasTexture.grow2D(renderer, atlasPacker.getResolution(), atlasPacker.getResolution(), Color::INVISIBLE);
				const glm::vec2 textureSize = atlasTexture.getSize();
				for (Sprite& sprite : sprites) {
					sprite.textureOffset = sprite.position / textureSize;
					sprite.textureScale = sprite.size / textureSize;
				}
			}
		} else {
			atlasTexture = {
				TextureInternalFormat::RGBA8,
				atlasPacker.getResolution(),
				atlasPacker.getResolution(),
				{.repeat = false, .useLinearFiltering = false, .useMipmap = false},
			};
			atlasTexture.fill2D(renderer, Color::INVISIBLE);
		}
	}

	AtlasPacker atlasPacker{};
	Texture atlasTexture{};
	std::vector<Sprite> sprites{};
};

} // namespace graphics
} // namespace donut

#endif
