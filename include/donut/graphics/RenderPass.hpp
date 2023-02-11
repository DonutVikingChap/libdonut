#ifndef DONUT_GRAPHICS_RENDER_PASS_HPP
#define DONUT_GRAPHICS_RENDER_PASS_HPP

#include <donut/Color.hpp>
#include <donut/LinearAllocator.hpp>
#include <donut/graphics/Font.hpp>
#include <donut/graphics/Model.hpp>
#include <donut/graphics/Shader2D.hpp>
#include <donut/graphics/Shader3D.hpp>
#include <donut/graphics/SpriteAtlas.hpp>
#include <donut/graphics/Texture.hpp>
#include <donut/graphics/TexturedQuad.hpp>

#include <array>                        // std::array
#include <compare>                      // std::strong_ordering
#include <cstddef>                      // std::byte, std::max_align_t
#include <glm/ext/matrix_transform.hpp> // glm::identity
#include <glm/glm.hpp>                  // glm::...
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
 *       the same RenderPass. Model instances with the same shader and model
 *       will be batched and rendered together, regardless of the order in which
 *       they are enqueued.
 * \note Instances that use different shaders will be rendered separately from
 *       each other, ordered by their Shader3DOptions::orderIndex. The built-in
 *       shader has an orderIndex of 0 by default, while custom shaders default
 *       to an orderIndex of 1.
 */
struct ModelInstance {
	/**
	 * Non-owning pointer the shader to use when rendering this model.
	 *
	 * \warning The pointed-to shader must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	Shader3D* shader = Shader3D::blinnPhongShader;

	/**
	 * Non-owning pointer to the model to be drawn.
	 *
	 * \warning The pointed-to model must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	const Model* model;

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
 * Configuration of a 2D textured quad instance, for drawing as part of a
 * RenderPass.
 *
 * Required fields:
 * - TextureInstance::texture
 *
 * \note Instances of this type will be rendered **after** any 3D instances in
 *       the same RenderPass. Consecutive quad instances with the same shader
 *       and texture will be batched and rendered together.
 *
 * \sa RectangleInstance
 * \sa QuadInstance
 * \sa SpriteInstance
 */
struct TextureInstance {
	/**
	 * Non-owning pointer to the shader to use when rendering this texture.
	 *
	 * \warning The pointed-to shader must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	Shader2D* shader = Shader2D::plainShader;

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
 * \note Instances of this type will be rendered **after** any 3D instances in
 *       the same RenderPass. Consecutive quad instances with the same shader
 *       and texture will be batched and rendered together.
 *
 * \sa TextureInstance
 * \sa QuadInstance
 * \sa SpriteInstance
 */
struct RectangleInstance {
	/**
	 * Non-owning pointer to the shader to use when rendering this rectangle.
	 *
	 * \warning The pointed-to shader must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	Shader2D* shader = Shader2D::plainShader;

	/**
	 * Non-owning pointer to a texture to apply to the rectangle.
	 *
	 * \warning The pointed-to texture must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	const Texture* texture = Texture::whiteR8G8B8A8Srgb1x1;

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
 * \note Instances of this type will be rendered **after** any 3D instances in
 *       the same RenderPass. Consecutive quad instances with the same shader
 *       and texture will be batched and rendered together.
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
	Shader2D* shader = Shader2D::plainShader;

	/**
	 * Non-owning pointer to a texture to apply to the quad.
	 *
	 * \warning The pointed-to texture must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	const Texture* texture = Texture::whiteR8G8B8A8Srgb1x1;

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
 * \note Instances of this type will be rendered **after** any 3D instances in
 *       the same RenderPass. Consecutive quad instances with the same shader
 *       and texture will be batched and rendered together.
 *
 * \sa TextureInstance
 * \sa RectangleInstance
 * \sa QuadInstance
 */
struct SpriteInstance {
	/**
	 * Non-owning pointer to the shader to use when rendering this sprite.
	 *
	 * \warning The pointed-to shader must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	Shader2D* shader = Shader2D::plainShader;

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
 * \note Instances of this type will be rendered **after** any 3D instances in
 *       the same RenderPass. Consecutive text instances with the same font will
 *       be batched and rendered together.
 */
struct TextInstance {
	/**
	 * Non-owning pointer to the shader to use when rendering the glyphs of this
	 * text.
	 *
	 * \warning The pointed-to shader must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	Shader2D* shader = Shader2D::alphaShader;

	/**
	 * Non-owning pointer to the font from which the text was shaped.
	 *
	 * \warning The pointed-to font must remain valid for the duration of its
	 *          use in the RenderPass, and must not be nullptr.
	 */
	const Font* font;

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
 */
class RenderPass {
public:
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

private:
	friend Renderer;

	struct ModelObjectInstancesFromModel {
		using allocator_type = LinearAllocator<std::vector<Model::Object::Instance, LinearAllocator<Model::Object::Instance>>>;

		Shader3D* shader;
		const Model* model;
		std::vector<std::vector<Model::Object::Instance, LinearAllocator<Model::Object::Instance>>, allocator_type> objectInstances;

		ModelObjectInstancesFromModel(ModelObjectInstancesFromModel&& other, const allocator_type& alloc) noexcept
			: shader(other.shader)
			, model(other.model)
			, objectInstances(other.objectInstances.begin(), other.objectInstances.end(), alloc) {}

		ModelObjectInstancesFromModel(Shader3D* shader, const Model* model, const allocator_type& alloc)
			: shader(shader)
			, model(model)
			, objectInstances(alloc) {}

		[[nodiscard]] std::strong_ordering operator<=>(const ModelObjectInstancesFromModel& other) const {
			const std::strong_ordering shaderOrdering = shader->options.orderIndex <=> other.shader->options.orderIndex;
			return (shaderOrdering != std::strong_ordering::equal) ? shaderOrdering : model <=> other.model;
		}

		[[nodiscard]] std::strong_ordering operator<=>(const ModelInstance& instance) const {
			const std::strong_ordering shaderOrdering = shader->options.orderIndex <=> instance.shader->options.orderIndex;
			return (shaderOrdering != std::strong_ordering::equal) ? shaderOrdering : model <=> instance.model;
		}
	};

	struct TexturedQuadInstances {
		using allocator_type = LinearAllocator<TexturedQuad::Instance>;

		Shader2D* shader;
		const Texture* texture;
		std::vector<TexturedQuad::Instance, allocator_type> instances;

		TexturedQuadInstances(TexturedQuadInstances&& other, const allocator_type& alloc) noexcept
			: shader(other.shader)
			, texture(other.texture)
			, instances(other.instances.begin(), other.instances.end(), alloc) {}

		TexturedQuadInstances(Shader2D* shader, const Texture* texture, const allocator_type& alloc)
			: shader(shader)
			, texture(texture)
			, instances(alloc) {}
	};

	alignas(std::max_align_t) std::array<std::byte, 1024> initialMemory;
	LinearMemoryResource memoryResource{initialMemory};
	std::vector<ModelObjectInstancesFromModel, LinearAllocator<ModelObjectInstancesFromModel>> objectsSortedByShaderAndModel{&memoryResource};
	std::vector<TexturedQuadInstances, LinearAllocator<TexturedQuadInstances>> quads{&memoryResource};
};

} // namespace graphics
} // namespace donut

#endif
