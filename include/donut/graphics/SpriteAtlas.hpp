#ifndef DONUT_GRAPHICS_SPRITE_ATLAS_HPP
#define DONUT_GRAPHICS_SPRITE_ATLAS_HPP

#include <donut/AtlasPacker.hpp>
#include <donut/Color.hpp>
#include <donut/graphics/Image.hpp>
#include <donut/graphics/Texture.hpp>

#include <cassert>     // assert
#include <cstddef>     // std::size_t
#include <cstdint>     // std::uint8_t
#include <glm/glm.hpp> // glm::...
#include <vector>      // std::vector

namespace donut {
namespace graphics {

class Renderer; // Forward declaration, to avoid including Renderer.hpp.

/**
 * Expandable texture atlas for packing 2D images into a spritesheet to
 * enable batch rendering.
 */
class SpriteAtlas {
public:
	/**
	 * Flags that describe how a sprite is flipped when rendered.
	 */
	using Flip = std::uint8_t;

	/**
	 * Flag values for Flip that describe how a sprite is flipped when rendered.
	 */
	enum FlipAxis : Flip {
		NO_FLIP = 0,                ///< Do not flip the sprite.
		FLIP_HORIZONTALLY = 1 << 0, ///< Flip the sprite along the X axis.
		FLIP_VERTICALLY = 1 << 1,   ///< Flip the sprite along the Y axis.
	};

	/**
	 * Identifier for a specific image in the spritesheet.
	 */
	struct SpriteId {
	private:
		friend SpriteAtlas;

		constexpr explicit SpriteId(std::size_t index) noexcept
			: index(index) {}

		std::size_t index;
	};

	/**
	 * Information about a specific image in the spritesheet.
	 */
	struct Sprite {
		glm::vec2 textureOffset{}; ///< Texture coordinate offset of the image in the texture atlas.
		glm::vec2 textureScale{};  ///< Texture coordinate scale of the image in the texture atlas.
		glm::vec2 position{};      ///< Position of the image in the texture atlas, in texels.
		glm::vec2 size{};          ///< Size of the image in the texture atlas, in texels.
	};

	/**
	 * Add a new image to the spritesheet, possibly expanding the texture atlas
	 * in order to make space for it.
	 *
	 * \param renderer renderer to use for expanding the texture atlas, if
	 *        needed.
	 * \param image non-owning view over the image to copy into the spritesheet.
	 * \param flip flags that describe how the sprite should be flipped when
	 *        rendered.
	 *
	 * \return an identifier for the inserted image.
	 *
	 * \throws graphics::Error on failure to copy the image or expand the
	 *         texture atlas.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \sa createSubSprite()
	 */
	[[nodiscard]] SpriteId insert(Renderer& renderer, const ImageView& image, Flip flip = NO_FLIP) {
		const auto [x, y, resized] = atlasPacker.insertRectangle(image.getWidth(), image.getHeight());
		prepareAtlasTexture(renderer, resized);

		atlasTexture.pasteImage2D(image, x, y);

		const glm::vec2 textureSize = atlasTexture.getSize2D();
		const glm::vec2 position{static_cast<float>(x), static_cast<float>(y)};
		const glm::vec2 size{static_cast<float>(image.getWidth()), static_cast<float>(image.getHeight())};
		glm::vec2 textureOffset{};
		glm::vec2 textureScale{};
		if ((flip & FLIP_HORIZONTALLY) != 0) {
			textureOffset.x = (position.x + size.x) / textureSize.x;
			textureScale.x = -size.x / textureSize.x;
		} else {
			textureOffset.x = position.x / textureSize.x;
			textureScale.x = size.x / textureSize.x;
		}
		if ((flip & FLIP_VERTICALLY) != 0) {
			textureOffset.y = (position.y + size.y) / textureSize.y;
			textureScale.y = -size.y / textureSize.y;
		} else {
			textureOffset.y = position.y / textureSize.y;
			textureScale.y = size.y / textureSize.y;
		}
		const std::size_t index = sprites.size();
		sprites.push_back(Sprite{
			.textureOffset = textureOffset,
			.textureScale = textureScale,
			.position = position,
			.size = size,
		});
		return SpriteId{index};
	}

	/**
	 * Add a new sprite that is defined as a sub-region of an existing sprite.
	 *
	 * \param baseSpriteId identifier for the existing sprite to create a
	 *        sub-region of. Must have been obtained from a previous call to
	 *        insert() or createSubSprite() on the same SpriteAtlas object as
	 *        the one that this function is called on.
	 * \param offsetX horizontal offset, in pixels, from the left edge of the
	 *        original sprite, where the new sprite will begin. Must be less
	 *        than or equal to the width of the original sprite image.
	 * \param offsetY vertical offset, in pixels, from the bottom edge of the
	 *        original sprite, where the new sprite will begin. Must be less
	 *        than or equal to the height of the original sprite image.
	 * \param width width, in pixels, of the new sprite region. Must be less
	 *        than or equal to the width of the original sprite image minus
	 *        offsetX.
	 * \param height height, in pixels, of the new sprite region. Must be less
	 *        than or equal to the height of the original sprite image minus
	 *        offsetY.
	 * \param flip flags that describe how the sprite should be flipped when
	 *        rendered.
	 *
	 * \return an identifier for the new sub-sprite.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note This function does not grow the texture atlas.
	 *
	 * \sa insert()
	 */
	[[nodiscard]] SpriteId createSubSprite(SpriteId baseSpriteId, std::size_t offsetX, std::size_t offsetY, std::size_t width, std::size_t height, Flip flip = NO_FLIP) {
		const Sprite& baseSprite = getSprite(baseSpriteId);
		assert(static_cast<float>(offsetX) <= baseSprite.size.x);
		assert(static_cast<float>(offsetY) <= baseSprite.size.y);
		assert(static_cast<float>(width) <= baseSprite.size.x - static_cast<float>(offsetX));
		assert(static_cast<float>(height) <= baseSprite.size.y - static_cast<float>(offsetY));

		const glm::vec2 textureSize = atlasTexture.getSize2D();
		const glm::vec2 position = baseSprite.position + glm::vec2{static_cast<float>(offsetX), static_cast<float>(offsetY)};
		const glm::vec2 size{static_cast<float>(width), static_cast<float>(height)};
		glm::vec2 textureOffset{};
		glm::vec2 textureScale{};
		if ((flip & FLIP_HORIZONTALLY) != 0) {
			textureOffset.x = (position.x + size.x) / textureSize.x;
			textureScale.x = -size.x / textureSize.x;
		} else {
			textureOffset.x = position.x / textureSize.x;
			textureScale.x = size.x / textureSize.x;
		}
		if ((flip & FLIP_VERTICALLY) != 0) {
			textureOffset.y = (position.y + size.y) / textureSize.y;
			textureScale.y = -size.y / textureSize.y;
		} else {
			textureOffset.y = position.y / textureSize.y;
			textureScale.y = size.y / textureSize.y;
		}
		const std::size_t index = sprites.size();
		sprites.push_back(Sprite{
			.textureOffset = textureOffset,
			.textureScale = textureScale,
			.position = position,
			.size = size,
		});
		return SpriteId{index};
	}

	/**
	 * Get information about a specific image in the spritesheet.
	 *
	 * \param id identifier for the image to get the information of. Must have
	 *        been obtained from a previous call to insert() or
	 *        createSubSprite() on the same SpriteAtlas object as the one that
	 *        this function is called on.
	 *
	 * \return a read-only reference to the sprite information that is valid
	 *         until the next call to insert(), or until the SpriteAtlas is
	 *         destroyed, whichever happens first.
	 */
	[[nodiscard]] const Sprite& getSprite(SpriteId id) const {
		assert(id.index < sprites.size());
		return sprites[id.index];
	}

	/**
	 * Get a reference to the internal texture atlas.
	 *
	 * \return a read-only reference to the texture atlas containing the
	 *         sprite image data that is valid until the next call to insert(),
	 *         or until the SpriteAtlas is destroyed, whichever happens first.
	 */
	[[nodiscard]] const Texture& getAtlasTexture() const noexcept {
		return atlasTexture;
	}

private:
	static constexpr std::size_t INITIAL_RESOLUTION = 128;
	static constexpr std::size_t PADDING = 6;

	void prepareAtlasTexture(Renderer& renderer, bool resized) {
		if (atlasTexture) {
			if (resized) {
				atlasTexture.grow2D(renderer, atlasPacker.getResolution(), atlasPacker.getResolution(), Color::INVISIBLE);
				const glm::vec2 textureSize = atlasTexture.getSize2D();
				for (Sprite& sprite : sprites) {
					if (sprite.textureScale.x < 0.0f) {
						sprite.textureOffset.x = (sprite.position.x + sprite.size.x) / textureSize.x;
						sprite.textureScale.x = -sprite.size.x / textureSize.x;
					} else {
						sprite.textureOffset.x = sprite.position.x / textureSize.x;
						sprite.textureScale.x = sprite.size.x / textureSize.x;
					}
					if (sprite.textureScale.y < 0.0f) {
						sprite.textureOffset.y = (sprite.position.y + sprite.size.y) / textureSize.y;
						sprite.textureScale.y = -sprite.size.y / textureSize.y;
					} else {
						sprite.textureOffset.y = sprite.position.y / textureSize.y;
						sprite.textureScale.y = sprite.size.y / textureSize.y;
					}
				}
			}
		} else {
			atlasTexture = {
				TextureFormat::R8G8B8A8_UNORM,
				atlasPacker.getResolution(),
				atlasPacker.getResolution(),
				{.repeat = false, .useLinearFiltering = false, .useMipmap = false},
			};
			atlasTexture.fill2D(renderer, Color::INVISIBLE);
		}
	}

	AtlasPacker<INITIAL_RESOLUTION, PADDING> atlasPacker{};
	Texture atlasTexture{};
	std::vector<Sprite> sprites{};
};

} // namespace graphics
} // namespace donut

#endif
