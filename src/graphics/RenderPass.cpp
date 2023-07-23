#include <donut/graphics/RenderPass.hpp>
#include <donut/math.hpp>

namespace donut::graphics {

RenderPass& RenderPass::draw(const ModelInstance& model) {
	assert(model.shader);
	assert(model.model);

	models.try_emplace(ModelInstances::Key{.shader = model.shader, .model = model.model}, &memoryResource)
		.first->second.instances.push_back(Model::Object::Instance{
			.transformation = model.transformation,
			.normalMatrix = inverseTranspose(mat3{model.transformation}),
			.tintColor = model.tintColor,
		});
	return *this;
}

RenderPass& RenderPass::draw(const TextureInstance& texture) {
	assert(texture.shader);
	assert(texture.texture);

	return draw(RectangleInstance{
		.shader = texture.shader,
		.texture = texture.texture,
		.position = texture.position,
		.size = texture.texture->getSize2D() * texture.scale,
		.angle = texture.angle,
		.origin = texture.origin,
		.textureOffset = texture.textureOffset,
		.textureScale = texture.textureScale,
		.tintColor = texture.tintColor,
	});
}

RenderPass& RenderPass::draw(const RectangleInstance& rectangle) {
	assert(rectangle.shader);

	return draw(QuadInstance{
		.shader = rectangle.shader,
		.texture = rectangle.texture,
		.transformation = translate(vec3{rectangle.position, 0.0f}) * //
	                      mat4{orientate2(rectangle.angle)} *         //
	                      scale(vec3{rectangle.size, 1.0f}) *         //
	                      translate(vec3{-rectangle.origin, 0.0f}),
		.textureOffset = rectangle.textureOffset,
		.textureScale = rectangle.textureScale,
		.tintColor = rectangle.tintColor,
	});
}

RenderPass& RenderPass::draw(const QuadInstance& quad) {
	assert(quad.shader);

	if (quads.empty() || last_quad->shader != quad.shader || last_quad->texture != quad.texture) {
		last_quad = quads.emplace_after(last_quad, quad.shader, quad.texture, &memoryResource);
	}
	last_quad->instances.push_back(TexturedQuad::Instance{
		.transformation = quad.transformation,
		.textureOffset = quad.textureOffset,
		.textureScale = quad.textureScale,
		.tintColor = quad.tintColor,
	});
	return *this;
}

RenderPass& RenderPass::draw(const SpriteInstance& sprite) {
	assert(sprite.shader);
	assert(sprite.atlas);

	const SpriteAtlas::Sprite& atlasSprite = sprite.atlas->getSprite(sprite.id);

	return draw(RectangleInstance{
		.shader = sprite.shader,
		.texture = &sprite.atlas->getAtlasTexture(),
		.position = sprite.position,
		.size = atlasSprite.size * sprite.scale,
		.angle = sprite.angle,
		.origin = sprite.origin,
		.textureOffset = atlasSprite.textureOffset,
		.textureScale = atlasSprite.textureScale,
		.tintColor = sprite.tintColor,
	});
}

RenderPass& RenderPass::draw(const TextInstance& text) {
	assert(text.shader);
	assert(text.font);

	const Texture* texture = &text.font->getAtlasTexture();
	if (quads.empty() || last_quad->shader != text.shader || last_quad->texture != texture) {
		last_quad = quads.emplace_after(last_quad, text.shader, texture, &memoryResource);
	}
	for (const Font::ShapedText::ShapedGlyph& shapedGlyph : text.text.shapedGlyphs) {
		last_quad->instances.push_back(TexturedQuad::Instance{
			.transformation = translate(vec3{text.position + shapedGlyph.offset, 0.0f}) * //
		                      scale(vec3{shapedGlyph.size, 1.0f}),
			.textureOffset = shapedGlyph.textureOffset,
			.textureScale = shapedGlyph.textureScale,
			.tintColor = text.color,
		});
	}
	return *this;
}

} // namespace donut::graphics
