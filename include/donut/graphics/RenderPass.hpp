#ifndef DONUT_GRAPHICS_RENDER_PASS_HPP
#define DONUT_GRAPHICS_RENDER_PASS_HPP

#include <donut/Color.hpp>
#include <donut/graphics/Font.hpp>
#include <donut/graphics/Model.hpp>
#include <donut/graphics/Shader2D.hpp>
#include <donut/graphics/Shader3D.hpp>
#include <donut/graphics/SpriteAtlas.hpp>
#include <donut/graphics/Texture.hpp>
#include <donut/graphics/TexturedQuad.hpp>

#include <cassert>                      // assert
#include <compare>                      // std::strong_ordering
#include <glm/ext/matrix_transform.hpp> // glm::identity
#include <glm/glm.hpp>                  // glm::...
#include <memory>                       // std::shared_ptr
#include <optional>                     // std::optional
#include <utility>                      // std::move
#include <vector>                       // std::vector

namespace donut {
namespace graphics {

/**
 * Configuration of a 3D Model instance, for drawing as part of a RenderPass.
 *
 * Required fields:
 * - ModelInstance::model
 *
 * \note Instances of this type will be rendered **before** any 2D instances in
 *       the same RenderPass.
 * \note Instances that use different shaders will be rendered separately from
 *       each other, ordered by their Shader3DOptions::orderIndex. The built-in
 *       shader has an orderIndex of 0 by default, while custom shaders default
 *       to an orderIndex of 1.
 */
struct ModelInstance {
	/**
	 * Shared pointer to a custom shader to use when rendering this model, or
	 * nullptr to use the model shader specified in the RendererOptions of the
	 * Renderer.
	 */
	std::shared_ptr<Shader3D> shader{};

	/**
	 * Shared pointer to the model to be drawn.
	 *
	 * \warning Must not be nullptr.
	 */
	std::shared_ptr<const Model> model;

	/**
	 * Transformation matrix to apply to every vertex position of the model, in
	 * world space.
	 */
	glm::mat4 transformation = glm::identity<glm::mat4>();

	/**
	 * Tint color to use in the shader.
	 *
	 * \note In the built-in shader, the output color is multiplied by this
	 *       value, meaning that a value of Color::WHITE, i.e. RGBA(1, 1, 1, 1)
	 *       in linear color, represents no modification to the original texture
	 *       color.
	 */
	Color tintColor = Color::WHITE;
};

/**
 * Configuration of a short-lived 2D textured quad instance, for drawing as part
 * of a RenderPass.
 *
 * Required fields:
 * - TransientTextureInstance::texture
 *
 * This type is meant for unique unmanaged textures that are unlikely to be
 * rendered again. This allows them to be rendered without any kind of
 * persistent batching or other optimization techniques which are
 * counter-productive in that case.
 *
 * For general drawing of textures across multiple frames, use TextureInstance,
 * RectangleInstance, QuadInstance or SpriteInstance instead.
 *
 * \note Instances of this type will be rendered **after** any 3D instances and
 *       **before** any other 2D instances in the same RenderPass.
 *
 * \sa TextureInstance
 * \sa RectangleInstance
 * \sa QuadInstance
 * \sa SpriteInstance
 */
struct TransientTextureInstance {
	/**
	 * Non-owning pointer to the texture to be drawn.
	 *
	 * \warning The pointed-to texture must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	const Texture* texture;

	/**
	 * Position, in world coordinates, to render the texture at, with respect to
	 * its TransientTextureInstance::origin.
	 */
	glm::vec2 position{0.0f, 0.0f};

	/**
	 * Coefficients to scale the size of the texture by.
	 *
	 * The resulting textured quad will have the size of the original texture,
	 * multiplied by this value.
	 */
	glm::vec2 scale{1.0f, 1.0f};

	/**
	 * Angle, in radians, to rotate the texture by, around its
	 * TransientTextureInstance::origin.
	 */
	float angle = 0.0f;

	/**
	 * Offset, in texture coordinates, specifying the origin relative to the
	 * bottom left of the texture. For example, a value of (0.5, 0.5) would
	 * represent the middle of the texture.
	 */
	glm::vec2 origin{0.0f, 0.0f};

	/**
	 * Offset, in texture coordinates, to apply to the texture coordinates
	 * before sampling the texture.
	 *
	 * \note This unscaled offset is applied after scaling the texture
	 *       coordinates by the TransientTextureInstance::textureScale.
	 */
	glm::vec2 textureOffset{0.0f, 0.0f};

	/**
	 * Coefficients to scale the texture coordinates by before sampling the
	 * texture.
	 *
	 * \note The texture coordinates are scaled before applying the unscaled
	 *       TransientTextureInstance::textureOffset.
	 */
	glm::vec2 textureScale{1.0f, 1.0f};

	/**
	 * Tint color to use in the shader.
	 *
	 * \note In the built-in shader, the output color is multiplied by this
	 *       value, meaning that a value of Color::WHITE, i.e. RGBA(1, 1, 1, 1)
	 *       in linear color, represents no modification to the original texture
	 *       color.
	 */
	Color tintColor = Color::WHITE;
};

/**
 * Configuration of a 2D textured quad instance, for drawing as part of a
 * RenderPass.
 *
 * Required fields:
 * - TextureInstance::texture
 *
 * \note Instances of this type will be rendered **after** any 3D instances or
 *       instances of type TransientTextureInstance, **together with** other
 *       non-sprite quad instances and **before** any sprite text instances in
 *       the same RenderPass.
 *
 * \sa RectangleInstance
 * \sa QuadInstance
 * \sa SpriteInstance
 */
struct TextureInstance {
	/**
	 * Shared pointer to the texture to be drawn.
	 *
	 * \warning Must not be nullptr.
	 */
	std::shared_ptr<const Texture> texture;

	/**
	 * Position, in world coordinates, to render the texture at, with respect to
	 * its TextureInstance::origin.
	 */
	glm::vec2 position{0.0f, 0.0f};

	/**
	 * Coefficients to scale the size of the texture by.
	 *
	 * The resulting textured quad will have the size of the original texture,
	 * multiplied by this value.
	 */
	glm::vec2 scale{1.0f, 1.0f};

	/**
	 * Angle, in radians, to rotate the texture by, around its
	 * TextureInstance::origin.
	 */
	float angle = 0.0f;

	/**
	 * Offset, in texture coordinates, specifying the origin relative to the
	 * bottom left of the texture. For example, a value of (0.5, 0.5) would
	 * represent the middle of the texture.
	 */
	glm::vec2 origin{0.0f, 0.0f};

	/**
	 * Offset, in texture coordinates, to apply to the texture coordinates
	 * before sampling the texture.
	 *
	 * \note This unscaled offset is applied after scaling the texture
	 *       coordinates by the TextureInstance::textureScale.
	 */
	glm::vec2 textureOffset{0.0f, 0.0f};

	/**
	 * Coefficients to scale the texture coordinates by before sampling the
	 * texture.
	 *
	 * \note The texture coordinates are scaled before applying the unscaled
	 *       TextureInstance::textureOffset.
	 */
	glm::vec2 textureScale{1.0f, 1.0f};

	/**
	 * Tint color to use in the shader.
	 *
	 * \note In the built-in shader, the output color is multiplied by this
	 *       value, meaning that a value of Color::WHITE, i.e. RGBA(1, 1, 1, 1)
	 *       in linear color, represents no modification to the original texture
	 *       color.
	 */
	Color tintColor = Color::WHITE;
};

/**
 * Configuration of a 2D rectangle instance, optionally textured, for drawing as
 * part of a RenderPass.
 *
 * \note Instances of this type will be rendered **after** any 3D instances or
 *       instances of type TransientTextureInstance, **together with** other
 *       non-sprite quad instances and **before** any sprite or text instances
 *       in the same RenderPass.
 *
 * \sa TextureInstance
 * \sa QuadInstance
 * \sa SpriteInstance
 */
struct RectangleInstance {
	/**
	 * Shared pointer to a texture to apply to the rectangle, or nullptr to use
	 * a solid white 1x1 texture.
	 */
	std::shared_ptr<const Texture> texture{};

	/**
	 * Position, in world coordinates, to render the rectangle at, with respect
	 * to its RectangleInstance::origin.
	 */
	glm::vec2 position{0.0f, 0.0f};

	/**
	 * Size of the rectangle, in world coordinates.
	 */
	glm::vec2 size{1.0f, 1.0f};

	/**
	 * Angle, in radians, to rotate the rectangle by, around its
	 * RectangleInstance::origin.
	 */
	float angle = 0.0f;

	/**
	 * Offset, in texture coordinates, specifying the origin relative to the
	 * bottom left of the rectangle. For example, a value of (0.5, 0.5) would
	 * represent the middle of the rectangle.
	 */
	glm::vec2 origin{0.0f, 0.0f};

	/**
	 * Offset, in texture coordinates, to apply to the texture coordinates
	 * before sampling the texture.
	 *
	 * \note This unscaled offset is applied after scaling the texture
	 *       coordinates by the RectangleInstance::textureScale.
	 */
	glm::vec2 textureOffset{0.0f, 0.0f};

	/**
	 * Coefficients to scale the texture coordinates by before sampling the
	 * texture.
	 *
	 * \note The texture coordinates are scaled before applying the unscaled
	 *       RectangleInstance::textureOffset.
	 */
	glm::vec2 textureScale{1.0f, 1.0f};

	/**
	 * Tint color to use in the shader.
	 *
	 * When no texture is specified, this controls the base color of the
	 * rectangle.
	 *
	 * \note In the built-in shader, the output color is multiplied by this
	 *       value, meaning that a value of Color::WHITE, i.e. RGBA(1, 1, 1, 1)
	 *       in linear color, represents no modification to the original texture
	 *       color.
	 */
	Color tintColor = Color::WHITE;
};

/**
 * Configuration of an arbitrarily shaded/transformed 2D quad instance,
 * optionally textured, for drawing as part of a RenderPass.
 *
 * \note Instances of this type will be rendered **after** any 3D instances or
 *       instances of type TransientTextureInstance, **together with** other
 *       non-sprite quad instances and **before** any sprite or text instances
 *       in the same RenderPass.
 * \note Instances that use different shaders will be rendered separately from
 *       each other, ordered by their Shader2DOptions::orderIndex. The built-in
 *       shader has an orderIndex of 0 by default, while custom shaders default
 *       to an orderIndex of 1.
 *
 * \sa TextureInstance
 * \sa RectangleInstance
 * \sa SpriteInstance
 */
struct QuadInstance {
	/**
	 * Shared pointer to a custom shader to use when rendering this quad, or
	 * nullptr to use the default shader specified in the RendererOptions of the
	 * Renderer.
	 */
	std::shared_ptr<Shader2D> shader{};

	/**
	 * Shared pointer to a texture to apply to the quad, or nullptr to use a
	 * solid white 1x1 texture.
	 */
	std::shared_ptr<const Texture> texture{};

	/**
	 * Transformation matrix to apply to every corner of the quad, in world
	 * space.
	 */
	glm::mat4 transformation = glm::identity<glm::mat4>();

	/**
	 * Offset, in texture coordinates, to apply to the texture coordinates
	 * before sampling the texture.
	 *
	 * \note This unscaled offset is applied after scaling the texture
	 *       coordinates by the QuadInstance::textureScale.
	 */
	glm::vec2 textureOffset{0.0f, 0.0f};

	/**
	 * Coefficients to scale the texture coordinates by before sampling the
	 * texture.
	 *
	 * \note The texture coordinates are scaled before applying the unscaled
	 *       QuadInstance::textureOffset.
	 */
	glm::vec2 textureScale{1.0f, 1.0f};

	/**
	 * Tint color to use in the shader.
	 *
	 * When no texture is specified, this controls the base color of the quad.
	 *
	 * \note In the built-in shader, the output color is multiplied by this
	 *       value, meaning that a value of Color::WHITE, i.e. RGBA(1, 1, 1, 1)
	 *       in linear color, represents no modification to the original texture
	 *       color.
	 */
	Color tintColor = Color::WHITE;
};

/**
 * Configuration of a 2D sprite instance from a SpriteAtlas, for drawing as part
 * of a RenderPass.
 *
 * Required fields:
 * - SpriteInstance::atlas
 * - SpriteInstance::id
 *
 * \note Instances of this type will be rendered **after** any 3D instances or
 *       non-sprite quad instances and **before** any text instances in the same
 *       RenderPass.
 *
 * \sa TextureInstance
 * \sa RectangleInstance
 * \sa QuadInstance
 */
struct SpriteInstance {
	/**
	 * Shared pointer to the texture atlas in which the sprite resides.
	 *
	 * \warning Must not be nullptr.
	 */
	std::shared_ptr<const SpriteAtlas> atlas;

	/**
	 * Identifier of the specific sprite in the SpriteInstance::atlas that is to
	 * be drawn.
	 *
	 * \warning Must be a valid sprite identifier obtained from the SpriteAtlas
	 *          pointed to by SpriteInstance::atlas.
	 */
	SpriteAtlas::SpriteId id;

	/**
	 * Position, in world coordinates, to render the sprite at, with respect to
	 * its SpriteInstance::origin.
	 */
	glm::vec2 position{0.0f, 0.0f};

	/**
	 * Coefficients to scale the size of the sprite by.
	 *
	 * The resulting textured quad will have the size of the original sprite,
	 * multiplied by this value.
	 */
	glm::vec2 scale{1.0f, 1.0f};

	/**
	 * Angle, in radians, to rotate the sprite by, around its
	 * SpriteInstance::origin.
	 */
	float angle = 0.0f;

	/**
	 * Offset, in texture coordinates, specifying the origin relative to the
	 * bottom left of the sprite. For example, a value of (0.5, 0.5) would
	 * represent the middle of the sprite.
	 */
	glm::vec2 origin{0.0f, 0.0f};

	/**
	 * Tint color to use in the shader.
	 *
	 * \note In the built-in shader, the output color is multiplied by this
	 *       value, meaning that a value of Color::WHITE, i.e. RGBA(1, 1, 1, 1)
	 *       in linear color, represents no modification to the original texture
	 *       color.
	 */
	Color tintColor = Color::WHITE;
};

/**
 * Configuration of a 2D instance of Font::ShapedText shaped from a Font, for
 * drawing as part of a RenderPass.
 *
 * Required fields:
 * - TextInstance::font
 * - TextInstance::text
 *
 * \note Instances of this type will be rendered **after** any 3D instances or
 *       other 2D instances in the same RenderPass.
 */
struct TextInstance {
	/**
	 * Shared pointer to the font from which the text was shaped.
	 *
	 * \warning Must not be nullptr.
	 */
	std::shared_ptr<const Font> font;

	/**
	 * Shaped text to be drawn, which should have been obtained from the font
	 * pointed to by TextInstance::font.
	 */
	Font::ShapedText text;

	/**
	 * Starting position, in world coordinates, to render the text at. This will
	 * be the first position on the baseline for the first line of text.
	 */
	glm::vec2 position{0.0f, 0.0f};

	/**
	 * Base text color.
	 */
	Color color = Color::WHITE;
};

/**
 * Graphics drawing queue for batch rendering using a Renderer.
 *
 * An instance of this class should generally be kept for multiple calls to
 * Renderer::render in order to allow allocated instance memory to be reused,
 * e.g. across frames. This can be done by calling RenderPass::reset() before
 * the draw calls for each render.
 */
class RenderPass {
public:
	/**
	 * Reset the render pass to a clear state.
	 *
	 * \note This preserves all of the memory allocated for instances drawn in
	 *       previous renders whose shared resources, such as textures and
	 *       fonts, are still alive, since they are likely to be used again in
	 *       subsequent renders.
	 * \note To reclaim all memory and force new allocations to be made for
	 *       subsequent draw calls, the RenderPass object can simply be
	 *       destroyed and replaced by a new object, e.g. by `renderPass = {}`.
	 */
	void reset() noexcept;

	/**
	 * Set the background color to use when clearing the framebuffer before any
	 * rendering takes place.
	 *
	 * \param color the background color to use, or an empty optional to not
	 *        clear the framebuffer color.
	 *
	 * \note When the render pass is rendered, if this background color is not
	 *       empty, it will clear the color of the entire targeted framebuffer
	 *       at the beginning of the render pass, regardless of what the
	 *       viewport region is set to. To clear only a specific region of the
	 *       framebuffer, draw a colored RectangleInstance covering that region
	 *       instead.
	 */
	void setBackgroundColor(std::optional<Color> color) {
		backgroundColor = color;
	}

	/**
	 * Enqueue a ModelInstance to be drawn when the render pass is rendered.
	 *
	 * \sa ModelInstance
	 */
	void draw(ModelInstance&& model);

	/**
	 * Enqueue a TransientTextureInstance to be drawn when the render pass is
	 * rendered.
	 *
	 * \sa TransientTextureInstance
	 */
	void draw(TransientTextureInstance&& transientTexture);

	/**
	 * Enqueue a TextureInstance to be drawn when the render pass is rendered.
	 *
	 * \sa TextureInstance
	 */
	void draw(TextureInstance&& texture);

	/**
	 * Enqueue a RectangleInstance to be drawn when the render pass is rendered.
	 *
	 * \sa RectangleInstance
	 */
	void draw(RectangleInstance&& rectangle);

	/**
	 * Enqueue a QuadInstance to be drawn when the render pass is rendered.
	 *
	 * \sa QuadInstance
	 */
	void draw(QuadInstance&& quad);

	/**
	 * Enqueue a SpriteInstance to be drawn when the render pass is rendered.
	 *
	 * \sa SpriteInstance
	 */
	void draw(SpriteInstance&& sprite);

	/**
	 * Enqueue a TextInstance to be drawn when the render pass is rendered.
	 *
	 * \sa TextInstance
	 */
	void draw(TextInstance&& text);

private:
	friend Renderer;

	template <typename Shader>
	[[nodiscard]] static std::strong_ordering compareShaders(const std::shared_ptr<Shader>& a, const std::shared_ptr<Shader>& b) noexcept {
		const int orderIndexA = (a) ? a->options.orderIndex : 0;
		const int orderIndexB = (b) ? b->options.orderIndex : 0;
		return orderIndexA <=> orderIndexB;
	}

	struct ModelObjectInstancesFromModel {
		std::shared_ptr<Shader3D> shader;
		std::shared_ptr<const Model> model;
		std::vector<std::vector<Model::Object::Instance>> objectInstances;

		[[nodiscard]] std::strong_ordering operator<=>(const ModelObjectInstancesFromModel& other) const {
			const std::strong_ordering shaderOrdering = compareShaders(shader, other.shader);
			return (shaderOrdering != std::strong_ordering::equal) ? shaderOrdering : model.get() <=> other.model.get();
		}

		[[nodiscard]] std::strong_ordering operator<=>(const ModelInstance& instance) const {
			const std::strong_ordering shaderOrdering = compareShaders(shader, instance.shader);
			return (shaderOrdering != std::strong_ordering::equal) ? shaderOrdering : model.get() <=> instance.model.get();
		}
	};

	struct TexturedQuadInstanceFromTransientTexture {
		const Texture* texture;
		TexturedQuad::Instance instance;
	};

	struct TexturedQuadInstancesFromQuad {
		std::shared_ptr<Shader2D> shader;
		std::shared_ptr<const Texture> texture;
		std::vector<TexturedQuad::Instance> instances;

		[[nodiscard]] std::strong_ordering operator<=>(const TexturedQuadInstancesFromQuad& other) const {
			const std::strong_ordering shaderOrdering = compareShaders(shader, other.shader);
			return (shaderOrdering != std::strong_ordering::equal) ? shaderOrdering : texture.get() <=> other.texture.get();
		}

		[[nodiscard]] std::strong_ordering operator<=>(const QuadInstance& instance) const {
			const std::strong_ordering shaderOrdering = compareShaders(shader, instance.shader);
			return (shaderOrdering != std::strong_ordering::equal) ? shaderOrdering : texture.get() <=> instance.texture.get();
		}
	};

	struct TexturedQuadInstancesFromSprite {
		std::shared_ptr<const SpriteAtlas> atlas;
		std::vector<TexturedQuad::Instance> instances;

		[[nodiscard]] std::strong_ordering operator<=>(const TexturedQuadInstancesFromSprite& other) const {
			return atlas.get() <=> other.atlas.get();
		}

		[[nodiscard]] std::strong_ordering operator<=>(const SpriteInstance& instance) const {
			return atlas.get() <=> instance.atlas.get();
		}
	};

	struct TexturedQuadInstancesFromText {
		std::shared_ptr<const Font> font;
		std::vector<TexturedQuad::Instance> instances;

		[[nodiscard]] std::strong_ordering operator<=>(const TexturedQuadInstancesFromText& other) const {
			return font.get() <=> other.font.get();
		}

		[[nodiscard]] std::strong_ordering operator<=>(const TextInstance& instance) const {
			return font.get() <=> instance.font.get();
		}
	};

	std::optional<Color> backgroundColor{};
	std::vector<ModelObjectInstancesFromModel> objectsSortedByShaderAndModel{};
	std::vector<TexturedQuadInstanceFromTransientTexture> transientTextures{};
	std::vector<TexturedQuadInstancesFromQuad> quadsSortedByShaderAndTexture{};
	std::vector<TexturedQuadInstancesFromSprite> spritesSortedByAtlas{};
	std::vector<TexturedQuadInstancesFromText> glyphsSortedByFont{};
};

} // namespace graphics
} // namespace donut

#endif
