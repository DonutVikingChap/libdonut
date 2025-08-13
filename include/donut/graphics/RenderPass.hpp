#ifndef DONUT_GRAPHICS_RENDER_PASS_HPP
#define DONUT_GRAPHICS_RENDER_PASS_HPP

#include <donut/Color.hpp>
#include <donut/LinearAllocator.hpp>
#include <donut/LinearBuffer.hpp>
#include <donut/graphics/Model.hpp>
#include <donut/graphics/Shader2D.hpp>
#include <donut/graphics/Shader3D.hpp>
#include <donut/graphics/SpriteAtlas.hpp>
#include <donut/graphics/Text.hpp>
#include <donut/graphics/Texture.hpp>
#include <donut/graphics/TexturedQuad.hpp>
#include <donut/math.hpp>

#include <cstddef>     // std::byte
#include <span>        // std::span
#include <string_view> // std::string_view, std::u8string_view
#include <vector>      // std::vector

namespace donut::graphics {

/**
 * Configuration of a 3D Model instance, for drawing as part of a RenderPass.
 *
 * Required fields:
 * - ModelInstance::model
 *
 * \note Consecutive 3D instances with the same shader and model will be batched
 *       and rendered together.
 */
struct ModelInstance {
	/**
	 * Non-owning pointer the shader to use when rendering this model.
	 *
	 * \warning The pointed-to shader must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	Shader3D* shader = Shader3D::BLINN_PHONG;

	/**
	 * Non-owning pointer to the model to be drawn.
	 *
	 * \warning The pointed-to model must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	const Model* model;

	/**
	 * Non-owning pointer to the texture to use for the base color, or nullptr
	 * to use the original textures specified by the model.
	 *
	 * \warning When not nullptr, the pointed-to texture must remain valid for
	 *          the duration of its use in the RenderPass.
	 */
	const Texture* diffuseMapOverride = nullptr;

	/**
	 * Non-owning pointer to the texture to use for specular highlights, or
	 * nullptr to use the original textures specified by the model.
	 *
	 * \warning When not nullptr, the pointed-to texture must remain valid for
	 *          the duration of its use in the RenderPass.
	 */
	const Texture* specularMapOverride = nullptr;

	/**
	 * Non-owning pointer to the texture to use for normal mapping, or nullptr
	 * to use the original textures specified by the model.
	 *
	 * \warning When not nullptr, the pointed-to texture must remain valid for
	 *          the duration of its use in the RenderPass.
	 */
	const Texture* normalMapOverride = nullptr;

	/**
	 * Non-owning pointer to the texture to use for emissive mapping, or nullptr
	 * to use the original textures specified by the model.
	 *
	 * \warning When not nullptr, the pointed-to texture must remain valid for
	 *          the duration of its use in the RenderPass.
	 */
	const Texture* emissiveMapOverride = nullptr;

	/**
	 * Transformation matrix to apply to every vertex position of the model, in
	 * world space.
	 */
	mat4 transformation = identity<mat4>();

	/**
	 * Offset, in texture coordinates, to apply to the texture coordinates
	 * before sampling textures.
	 *
	 * \note This unscaled offset is applied after scaling the texture
	 *       coordinates by the ModelInstance::textureScale.
	 */
	vec2 textureOffset{0.0f, 0.0f};

	/**
	 * Coefficients to scale the texture coordinates by before sampling
	 * textures.
	 *
	 * \note The texture coordinates are scaled before applying the unscaled
	 *       ModelInstance::textureOffset.
	 */
	vec2 textureScale{1.0f, 1.0f};

	/**
	 * Tint color to use in the shader.
	 *
	 * \note In the default shader, the output color is multiplied by this
	 *       value, meaning that a value of Color::WHITE, i.e. RGBA(1, 1, 1, 1)
	 *       in linear color, represents no modification to the original texture
	 *       color.
	 */
	Color tintColor = Color::WHITE;

	/**
	 * Specular factor to use in the shader.
	 *
	 * \note In the default shader, the specular color is multiplied by this
	 *       value, meaning that a value of (1, 1, 1) represents no modification
	 *       to the original specular map color.
	 */
	vec3 specularFactor{1.0f, 1.0f, 1.0f};

	/**
	 * Emissive factor to use in the shader.
	 *
	 * \note In the default shader, the emissive color is multiplied by this
	 *       value, meaning that a value of (1, 1, 1) represents no modification
	 *       to the original emissive map color.
	 */
	vec3 emissiveFactor{1.0f, 1.0f, 1.0f};
};

/**
 * Configuration of an arbitrarily shaded/transformed 2D quad instance,
 * optionally textured, for drawing as part of a RenderPass.
 *
 * \note Consecutive 2D instances with the same shader and texture will be
 *       batched and rendered together.
 *
 * \sa TextureInstance
 * \sa RectangleInstance
 * \sa SpriteInstance
 */
struct QuadInstance {
	/**
	 * Non-owning pointer to the shader to use when rendering this quad.
	 *
	 * \warning The pointed-to shader must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	Shader2D* shader = Shader2D::PLAIN;

	/**
	 * Non-owning pointer to a texture to apply to the quad.
	 *
	 * \warning The pointed-to texture must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	const Texture* texture = Texture::WHITE;

	/**
	 * Transformation matrix to apply to every corner of the quad.
	 */
	mat3 transformation = identity<mat3>();

	/**
	 * Offset, in texture coordinates, to apply to the texture coordinates
	 * before sampling the texture.
	 *
	 * \note This unscaled offset is applied after scaling the texture
	 *       coordinates by the QuadInstance::textureScale.
	 */
	vec2 textureOffset{0.0f, 0.0f};

	/**
	 * Coefficients to scale the texture coordinates by before sampling the
	 * texture.
	 *
	 * \note The texture coordinates are scaled before applying the unscaled
	 *       QuadInstance::textureOffset.
	 */
	vec2 textureScale{1.0f, 1.0f};

	/**
	 * Tint color to use in the shader.
	 *
	 * When no texture is specified, this controls the base color of the quad.
	 *
	 * \note In the default shader, the output color is multiplied by this
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
 * \note Consecutive 2D instances with the same shader and texture will be
 *       batched and rendered together.
 *
 * \sa QuadInstance
 * \sa RectangleInstance
 * \sa SpriteInstance
 */
struct TextureInstance {
	/**
	 * Non-owning pointer to the shader to use when rendering this texture.
	 *
	 * \warning The pointed-to shader must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	Shader2D* shader = Shader2D::PLAIN;

	/**
	 * Non-owning pointer to the texture to be drawn.
	 *
	 * \warning The pointed-to texture must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	const Texture* texture;

	/**
	 * Position, in world coordinates, to render the texture at, with respect to
	 * its TextureInstance::origin.
	 */
	vec2 position{0.0f, 0.0f};

	/**
	 * Coefficients to scale the size of the texture by.
	 *
	 * The resulting textured quad will have the size of the original texture,
	 * multiplied by this value.
	 */
	vec2 scale{1.0f, 1.0f};

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
	vec2 origin{0.0f, 0.0f};

	/**
	 * Offset, in texture coordinates, to apply to the texture coordinates
	 * before sampling the texture.
	 *
	 * \note This unscaled offset is applied after scaling the texture
	 *       coordinates by the TextureInstance::textureScale.
	 */
	vec2 textureOffset{0.0f, 0.0f};

	/**
	 * Coefficients to scale the texture coordinates by before sampling the
	 * texture.
	 *
	 * \note The texture coordinates are scaled before applying the unscaled
	 *       TextureInstance::textureOffset.
	 */
	vec2 textureScale{1.0f, 1.0f};

	/**
	 * Tint color to use in the shader.
	 *
	 * \note In the default shader, the output color is multiplied by this
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
 * \note Consecutive 2D instances with the same shader and texture will be
 *       batched and rendered together.
 *
 * \sa QuadInstance
 * \sa TextureInstance
 * \sa SpriteInstance
 */
struct RectangleInstance {
	/**
	 * Non-owning pointer to the shader to use when rendering this rectangle.
	 *
	 * \warning The pointed-to shader must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	Shader2D* shader = Shader2D::PLAIN;

	/**
	 * Non-owning pointer to a texture to apply to the rectangle.
	 *
	 * \warning The pointed-to texture must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	const Texture* texture = Texture::WHITE;

	/**
	 * Position, in world coordinates, to render the rectangle at, with respect
	 * to its RectangleInstance::origin.
	 */
	vec2 position{0.0f, 0.0f};

	/**
	 * Size of the rectangle, in world coordinates.
	 */
	vec2 size{1.0f, 1.0f};

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
	vec2 origin{0.0f, 0.0f};

	/**
	 * Offset, in texture coordinates, to apply to the texture coordinates
	 * before sampling the texture.
	 *
	 * \note This unscaled offset is applied after scaling the texture
	 *       coordinates by the RectangleInstance::textureScale.
	 */
	vec2 textureOffset{0.0f, 0.0f};

	/**
	 * Coefficients to scale the texture coordinates by before sampling the
	 * texture.
	 *
	 * \note The texture coordinates are scaled before applying the unscaled
	 *       RectangleInstance::textureOffset.
	 */
	vec2 textureScale{1.0f, 1.0f};

	/**
	 * Tint color to use in the shader.
	 *
	 * When no texture is specified, this controls the base color of the
	 * rectangle.
	 *
	 * \note In the default shader, the output color is multiplied by this
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
 * \note Consecutive sprite instances with the same shader and atlas will be
 *       batched and rendered together.
 *
 * \sa QuadInstance
 * \sa TextureInstance
 * \sa RectangleInstance
 */
struct SpriteInstance {
	/**
	 * Non-owning pointer to the shader to use when rendering this sprite.
	 *
	 * \warning The pointed-to shader must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	Shader2D* shader = Shader2D::PLAIN;

	/**
	 * Non-owning pointer to the texture atlas in which the sprite resides.
	 *
	 * \warning The pointed-to atlas must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	const SpriteAtlas* atlas;

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
	vec2 position{0.0f, 0.0f};

	/**
	 * Coefficients to scale the size of the sprite by.
	 *
	 * The resulting textured quad will have the size of the original sprite,
	 * multiplied by this value.
	 */
	vec2 scale{1.0f, 1.0f};

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
	vec2 origin{0.0f, 0.0f};

	/**
	 * Tint color to use in the shader.
	 *
	 * \note In the default shader, the output color is multiplied by this
	 *       value, meaning that a value of Color::WHITE, i.e. RGBA(1, 1, 1, 1)
	 *       in linear color, represents no modification to the original texture
	 *       color.
	 */
	Color tintColor = Color::WHITE;
};

/**
 * Configuration of a 2D instance of Text shaped from a Font, for drawing as
 * part of a RenderPass.
 *
 * Required fields:
 * - TextInstance::text
 *
 * \note Consecutive text instances with the same shader and font will be
 *       batched and rendered together.
 *
 * \sa TextCopyInstance
 * \sa TextUTF8StringInstance
 * \sa TextStringInstance
 */
struct TextInstance {
	/**
	 * Non-owning pointer to the shader to use when rendering the glyphs of this
	 * text.
	 *
	 * \warning The pointed-to shader must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	Shader2D* shader = Shader2D::ALPHA;

	/**
	 * Non-owning read-only pointer to the shaped text to draw.
	 *
	 * \warning The pointed-to text, as well as all of the fonts used by it,
	 *          must remain valid for the duration of its use in the RenderPass,
	 *          and must not be nullptr.
	 */
	const Text* text;

	/**
	 * Starting position, in world coordinates, to render the text at. This will
	 * be the first position on the baseline of the first line of text.
	 */
	vec2 position{0.0f, 0.0f};

	/**
	 * Base text color.
	 */
	Color color = Color::WHITE;
};

/**
 * Configuration of a copied 2D instance of Text shaped from a Font, for drawing
 * as part of a RenderPass.
 *
 * Required fields:
 * - TextInstance::text
 *
 * \note Consecutive text instances with the same shader and font will be
 *       batched and rendered together.
 * \note Unlike TextInstance, this instance type does not require the given text
 *       to remain valid for the duration of its use in the RenderPass, since
 *       the RenderPass stores a copy of the text when it is drawn.
 *
 * \sa TextInstance
 * \sa TextUTF8StringInstance
 * \sa TextStringInstance
 */
struct TextCopyInstance {
	/**
	 * Non-owning pointer to the shader to use when rendering the glyphs of this
	 * text.
	 *
	 * \warning The pointed-to shader must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	Shader2D* shader = Shader2D::ALPHA;

	/**
	 * Non-owning read-only pointer to the shaped text to copy, and later draw.
	 *
	 * \warning All of the fonts used by the text must remain valid for the
	 *          duration of their use in the RenderPass.
	 * \warning Must not be nullptr.
	 */
	const Text* text;

	/**
	 * Starting position, in world coordinates, to render the text at. This will
	 * be the first position on the baseline of the first line of text.
	 */
	vec2 position{0.0f, 0.0f};

	/**
	 * Base text color.
	 */
	Color color = Color::WHITE;
};

/**
 * Configuration of a 2D instance of a UTF-8 string of text with a Font, for
 * drawing as part of a RenderPass.
 *
 * Required fields:
 * - TextUTF8StringInstance::font
 * - TextUTF8StringInstance::characterSize
 * - TextUTF8StringInstance::string
 *
 * \note Consecutive text instances with the same shader and font will be
 *       batched and rendered together.
 *
 * \sa TextInstance
 * \sa TextStringInstance
 */
struct TextUTF8StringInstance {
	/**
	 * Non-owning pointer to the shader to use when rendering the glyphs of this
	 * text.
	 *
	 * \warning The pointed-to shader must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	Shader2D* shader = Shader2D::ALPHA;

	/**
	 * Non-owning pointer to the font from which to shape the text.
	 *
	 * \warning The pointed-to font must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	Font* font;

	/**
	 * Character size to shape the glyphs at.
	 */
	u32 characterSize;

	/**
	 * Starting position, in world coordinates, to render the text at. This will
	 * be the first position on the baseline of the first line of text.
	 */
	vec2 position{0.0f, 0.0f};

	/**
	 * Scaling to apply to the size of the shaped glyphs. The result is affected
	 * by FontOptions::useLinearFiltering.
	 *
	 * \remark The best visual results are usually achieved when the text is
	 *         shaped at an appropriate character size to begin with, rather
	 *         than relying on this scale parameter. As such, the scale should
	 *         generally be kept at (1, 1) unless many different character sizes
	 *         are used with this font and there is a strict requirement on the
	 *         maximum size of the texture atlas.
	 */
	vec2 scale{1.0f, 1.0f};

	/**
	 * Offset, in normalized coordinates, specifying the origin relative to the
	 * baseline of the first line of text. For example, a value of (0.5, 0.5)
	 * would represent the middle of the first line of text.
	 *
	 * \remark A value of (0.5, 0.0) can be used to center text on the X axis.
	 */
	vec2 origin{0.0f, 0.0f};

	/**
	 * Base text color.
	 */
	Color color = Color::WHITE;

	/**
	 * UTF8-encoded string to shape the text from.
	 *
	 * \note Right-to-left text shaping is currently not supported.
	 * \note Grapheme clusters are currently not supported, and may be rendered
	 *       incorrectly. Only one Unicode code point is rendered at a time.
	 *
	 * \warning If the string contains invalid UTF-8, the invalid code points
	 *          will generate unspecified glyphs that may have any appearance.
	 */
	std::u8string_view string;
};

/**
 * Configuration of a 2D instance of a string of text with a Font, for drawing
 * as part of a RenderPass.
 *
 * Required fields:
 * - TextStringInstance::font
 * - TextStringInstance::characterSize
 * - TextStringInstance::string
 *
 * \note Consecutive text instances with the same shader and font will be
 *       batched and rendered together.
 *
 * \sa TextInstance
 * \sa TextUTF8StringInstance
 */
struct TextStringInstance {
	/**
	 * Non-owning pointer to the shader to use when rendering the glyphs of this
	 * text.
	 *
	 * \warning The pointed-to shader must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	Shader2D* shader = Shader2D::ALPHA;

	/**
	 * Non-owning pointer to the font from which to shape the text.
	 *
	 * \warning The pointed-to font must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	Font* font;

	/**
	 * Character size to shape the glyphs at.
	 */
	u32 characterSize;

	/**
	 * Starting position, in world coordinates, to render the text at. This will
	 * be the first position on the baseline of the first line of text.
	 */
	vec2 position{0.0f, 0.0f};

	/**
	 * Scaling to apply to the size of the shaped glyphs. The result is affected
	 * by FontOptions::useLinearFiltering.
	 *
	 * \remark The best visual results are usually achieved when the text is
	 *         shaped at an appropriate character size to begin with, rather
	 *         than relying on this scale parameter. As such, the scale should
	 *         generally be kept at (1, 1) unless many different character sizes
	 *         are used with this font and there is a strict requirement on the
	 *         maximum size of the texture atlas.
	 */
	vec2 scale{1.0f, 1.0f};

	/**
	 * Offset, in normalized coordinates, specifying the origin relative to the
	 * baseline of the first line of text. For example, a value of (0.5, 0.5)
	 * would represent the middle of the first line of text.
	 *
	 * \remark A value of (0.5, 0.0) can be used to center text on the X axis.
	 */
	vec2 origin{0.0f, 0.0f};

	/**
	 * Base text color.
	 */
	Color color = Color::WHITE;

	/**
	 * String to shape the text from.
	 *
	 * The string will be interpreted as containing UTF-8-encoded text.
	 *
	 * \note Right-to-left text shaping is currently not supported.
	 * \note Grapheme clusters are currently not supported, and may be rendered
	 *       incorrectly. Only one Unicode code point is rendered at a time.
	 *
	 * \warning If the string contains invalid UTF-8, the invalid code points
	 *          will generate unspecified glyphs that may have any appearance.
	 */
	std::string_view string;
};

/**
 * Graphics drawing queue for batch rendering using a Renderer.
 */
class RenderPass {
public:
	/**
	 * Construct an empty RenderPass.
	 */
	RenderPass() noexcept = default;

	/**
	 * Construct an empty RenderPass with some initial storage pre-allocated.
	 *
	 * \param initialMemory non-owning view over a contiguous chunk of available
	 *        memory that the RenderPass may use as temporary storage.
	 *
	 * \warning The pointed-to memory must remain valid until the RenderPass has
	 *          been destroyed.
	 */
	RenderPass(std::span<std::byte> initialMemory) noexcept
		: memoryResource(initialMemory) {}

	/**
	 * Enqueue a ModelInstance to be drawn when the render pass is rendered.
	 *
	 * \return `*this`, for chaining.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \sa ModelInstance
	 */
	RenderPass& draw(const ModelInstance& model);

	/**
	 * Enqueue a QuadInstance to be drawn when the render pass is rendered.
	 *
	 * \return `*this`, for chaining.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \sa QuadInstance
	 */
	RenderPass& draw(const QuadInstance& quad);

	/**
	 * Enqueue a TextureInstance to be drawn when the render pass is rendered.
	 *
	 * \return `*this`, for chaining.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \sa TextureInstance
	 */
	RenderPass& draw(const TextureInstance& texture);

	/**
	 * Enqueue a RectangleInstance to be drawn when the render pass is rendered.
	 *
	 * \return `*this`, for chaining.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \sa RectangleInstance
	 */
	RenderPass& draw(const RectangleInstance& rectangle);

	/**
	 * Enqueue a SpriteInstance to be drawn when the render pass is rendered.
	 *
	 * \return `*this`, for chaining.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \sa SpriteInstance
	 */
	RenderPass& draw(const SpriteInstance& sprite);

	/**
	 * Enqueue a TextInstance to be drawn when the render pass is rendered.
	 *
	 * \return `*this`, for chaining.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \sa TextInstance
	 */
	RenderPass& draw(const TextInstance& text);

	/**
	 * Enqueue a TextCopyInstance to be drawn when the render pass is rendered.
	 *
	 * \return `*this`, for chaining.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \sa TextCopyInstance
	 */
	RenderPass& draw(const TextCopyInstance& text);

	/**
	 * Enqueue a TextUTF8StringInstance to be drawn when the render pass is
	 * rendered.
	 *
	 * \return `*this`, for chaining.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \sa TextUTF8StringInstance
	 */
	RenderPass& draw(const TextUTF8StringInstance& text);

	/**
	 * Enqueue a TextStringInstance to be drawn when the render pass is
	 * rendered.
	 *
	 * \return `*this`, for chaining.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \sa TextStringInstance
	 */
	RenderPass& draw(const TextStringInstance& text);

private:
	friend Renderer;

	struct CommandUseShader3D {
		Shader3D* shader;
	};

	struct CommandUseShader2D {
		Shader2D* shader;
	};

	struct CommandUseModel {
		const Model* model;
		const Texture* diffuseMapOverride;
		const Texture* specularMapOverride;
		const Texture* normalMapOverride;
		const Texture* emissiveMapOverride;
	};

	struct CommandUseTexture {
		const Texture* texture;
	};

	struct CommandUseSpriteAtlas {
		const SpriteAtlas* atlas;
	};

	struct CommandUseFont {
		Font* font;
	};

	struct CommandDrawModelInstance {
		mat4 transformation;
		Color tintColor;
		vec2 textureOffset;
		vec2 textureScale;
		vec3 specularFactor;
		vec3 emissiveFactor;
	};

	struct CommandDrawQuadInstance {
		mat3 transformation;
		Color tintColor;
		vec2 textureOffset;
		vec2 textureScale;
	};

	struct CommandDrawTextureInstance {
		Color tintColor;
		vec2 position;
		vec2 scale;
		vec2 origin;
		vec2 textureOffset;
		vec2 textureScale;
		float angle;
	};

	struct CommandDrawRectangleInstance {
		Color tintColor;
		vec2 position;
		vec2 size;
		vec2 origin;
		vec2 textureOffset;
		vec2 textureScale;
		float angle;
	};

	struct CommandDrawSpriteInstance {
		Color tintColor;
		vec2 position;
		vec2 scale;
		vec2 origin;
		float angle;
		SpriteAtlas::SpriteId id;
	};

	struct CommandDrawTextInstance {
		Color color;
		const Text* text;
		vec2 position;
	};

	struct CommandDrawTextCopyInstance {
		Color color;
		std::span<const Text::ShapedGlyph> shapedGlyphs;
		vec2 position;
	};

	struct CommandDrawTextStringInstance {
		Color color;
		std::string_view string;
		vec2 position;
		vec2 scale;
		vec2 origin;
		u32 characterSize;
	};

	LinearMemoryResource memoryResource{};
	LinearBuffer<                      //
		CommandUseShader3D,            //
		CommandUseShader2D,            //
		CommandUseModel,               //
		CommandUseTexture,             //
		CommandUseSpriteAtlas,         //
		CommandUseFont,                //
		CommandDrawModelInstance,      //
		CommandDrawQuadInstance,       //
		CommandDrawTextureInstance,    //
		CommandDrawRectangleInstance,  //
		CommandDrawSpriteInstance,     //
		CommandDrawTextInstance,       //
		CommandDrawTextCopyInstance,   //
		CommandDrawTextStringInstance, //
		Text::ShapedGlyph[],           //
		char[]>
		commandBuffer{&memoryResource, memoryResource.getRemainingCapacity()};
	std::vector<Font*, LinearAllocator<Font*>> fonts{&memoryResource};
	Shader3D* previousShader3D = nullptr;
	Shader2D* previousShader2D = nullptr;
	const Model* previousModel = nullptr;
	const Texture* previousDiffuseMapOverride = nullptr;
	const Texture* previousSpecularMapOverride = nullptr;
	const Texture* previousNormalMapOverride = nullptr;
	const Texture* previousEmissiveMapOverride = nullptr;
	const Texture* previousTexture = nullptr;
	const SpriteAtlas* previousSpriteAtlas = nullptr;
	Font* previousFont = nullptr;
};

} // namespace donut::graphics

#endif
