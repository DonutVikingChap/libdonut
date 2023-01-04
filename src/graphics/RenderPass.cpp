#include <donut/graphics/RenderPass.hpp>

#include <algorithm>                    // std::lower_bound
#include <cstddef>                      // std::size_t
#include <glm/ext/matrix_transform.hpp> // glm::identity, glm::translate, glm::rotate, glm::scale
#include <glm/glm.hpp>                  // glm::...
#include <glm/gtc/matrix_inverse.hpp>   // glm::inverseTranspose
#include <memory_resource>              // std::pmr::...
#include <vector>                       // std::vector

namespace donut {
namespace graphics {

RenderPass& RenderPass::draw(const ModelInstance& model) {
	assert(model.model);

	const glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3{model.transformation});

	const std::size_t objectCount = model.model->objects.size();

	std::pmr::vector<std::pmr::vector<Model::Object::Instance>>* objectInstances = nullptr;
	const auto it = std::lower_bound(objectsSortedByShaderAndModel.begin(), objectsSortedByShaderAndModel.end(), model);
	if (it == objectsSortedByShaderAndModel.end()) {
		objectInstances = &objectsSortedByShaderAndModel.emplace_back(model.shader, model.model).objectInstances;
		objectInstances->resize(objectCount);
	} else if (it->shader == model.shader && it->model == model.model) {
		objectInstances = &it->objectInstances;
		assert(objectInstances->size() == objectCount);
	} else {
		objectInstances = &objectsSortedByShaderAndModel.emplace(it, model.shader, model.model)->objectInstances;
		objectInstances->resize(objectCount);
	}
	for (std::size_t i = 0; i < objectCount; ++i) {
		(*objectInstances)[i].push_back({
			.transformation = model.transformation,
			.normalMatrix = normalMatrix,
			.tintColor = model.tintColor,
		});
	}
	return *this;
}

RenderPass& RenderPass::draw(const TextureInstance& texture) {
	assert(texture.texture);

	draw(RectangleInstance{
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
	return *this;
}

RenderPass& RenderPass::draw(const RectangleInstance& rectangle) {
	glm::mat4 transformation = glm::identity<glm::mat4>();
	transformation = glm::translate(transformation, {rectangle.position, 0.0f});
	transformation = glm::rotate(transformation, rectangle.angle, {0.0f, 0.0f, 1.0f});
	transformation = glm::scale(transformation, {rectangle.size, 1.0f});
	transformation = glm::translate(transformation, {-rectangle.origin, 0.0f});
	draw(QuadInstance{
		.shader = rectangle.shader,
		.texture = rectangle.texture,
		.transformation = transformation,
		.textureOffset = rectangle.textureOffset,
		.textureScale = rectangle.textureScale,
		.tintColor = rectangle.tintColor,
	});
	return *this;
}

RenderPass& RenderPass::draw(const QuadInstance& quad) {
	const TexturedQuad::Instance instance{
		.transformation = quad.transformation,
		.textureOffset = quad.textureOffset,
		.textureScale = quad.textureScale,
		.tintColor = quad.tintColor,
	};

	const auto it = std::lower_bound(quadsSortedByShaderAndTexture.begin(), quadsSortedByShaderAndTexture.end(), quad);
	if (it == quadsSortedByShaderAndTexture.end()) {
		quadsSortedByShaderAndTexture.emplace_back(quad.shader, quad.texture).instances.push_back(instance);
	} else if (it->shader == quad.shader && it->texture == quad.texture) {
		it->instances.push_back(instance);
	} else {
		quadsSortedByShaderAndTexture.emplace(it, quad.shader, quad.texture)->instances.push_back(instance);
	}
	return *this;
}

RenderPass& RenderPass::draw(const SpriteInstance& sprite) {
	assert(sprite.atlas);

	const SpriteAtlas::Sprite& atlasSprite = sprite.atlas->getSprite(sprite.id);

	draw(RectangleInstance{
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
	return *this;
}

RenderPass& RenderPass::draw(const TextInstance& text) {
	assert(text.font);

	std::pmr::vector<TexturedQuad::Instance>* instances = nullptr;
	const auto it = std::lower_bound(glyphsSortedByFont.begin(), glyphsSortedByFont.end(), text);
	if (it == glyphsSortedByFont.end()) {
		instances = &glyphsSortedByFont.emplace_back(text.font).instances;
	} else if (it->font == text.font) {
		instances = &it->instances;
	} else {
		instances = &glyphsSortedByFont.emplace(it, text.font)->instances;
	}
	for (const Font::ShapedText::ShapedGlyph& shapedGlyph : text.text.shapedGlyphs) {
		instances->push_back({
			.transformation = glm::scale(glm::translate(glm::identity<glm::mat4>(), {text.position + shapedGlyph.offset, 0.0f}), {shapedGlyph.size, 1.0f}),
			.textureOffset = shapedGlyph.textureOffset,
			.textureScale = shapedGlyph.textureScale,
			.tintColor = text.color,
		});
	}
	return *this;
}

} // namespace graphics
} // namespace donut
