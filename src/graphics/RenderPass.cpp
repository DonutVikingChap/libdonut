#include <donut/graphics/RenderPass.hpp>

#include <algorithm>                    // std::lower_bound
#include <cstddef>                      // std::size_t
#include <glm/ext/matrix_transform.hpp> // glm::identity, glm::translate, glm::rotate, glm::scale
#include <glm/glm.hpp>                  // glm::...
#include <glm/gtc/matrix_inverse.hpp>   // glm::inverseTranspose
#include <vector>                       // std::vector

namespace donut {
namespace graphics {

RenderPass& RenderPass::draw(const ModelInstance& model) {
	assert(model.shader);
	assert(model.model);

	const glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3{model.transformation});

	const std::size_t objectCount = model.model->objects.size();

	decltype(ModelObjectInstancesFromModel::objectInstances)* objectInstances = nullptr;
	const auto it = std::lower_bound(objectsSortedByShaderAndModel.begin(), objectsSortedByShaderAndModel.end(), model);
	if (it == objectsSortedByShaderAndModel.end()) {
		objectInstances = &objectsSortedByShaderAndModel.emplace_back(model.shader, model.model, &memoryResource).objectInstances;
		objectInstances->resize(objectCount, std::vector<Model::Object::Instance, LinearAllocator<Model::Object::Instance>>{&memoryResource});
	} else if (it->shader == model.shader && it->model == model.model) {
		objectInstances = &it->objectInstances;
		assert(objectInstances->size() == objectCount);
	} else {
		objectInstances = &objectsSortedByShaderAndModel.emplace(it, model.shader, model.model, &memoryResource)->objectInstances;
		objectInstances->resize(objectCount, std::vector<Model::Object::Instance, LinearAllocator<Model::Object::Instance>>{&memoryResource});
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

	glm::mat4 transformation = glm::identity<glm::mat4>();
	transformation = glm::translate(transformation, {rectangle.position, 0.0f});
	transformation = glm::rotate(transformation, rectangle.angle, {0.0f, 0.0f, 1.0f});
	transformation = glm::scale(transformation, {rectangle.size, 1.0f});
	transformation = glm::translate(transformation, {-rectangle.origin, 0.0f});
	return draw(QuadInstance{
		.shader = rectangle.shader,
		.texture = rectangle.texture,
		.transformation = transformation,
		.textureOffset = rectangle.textureOffset,
		.textureScale = rectangle.textureScale,
		.tintColor = rectangle.tintColor,
	});
}

RenderPass& RenderPass::draw(const QuadInstance& quad) {
	assert(quad.shader);

	const TexturedQuad::Instance instance{
		.transformation = quad.transformation,
		.textureOffset = quad.textureOffset,
		.textureScale = quad.textureScale,
		.tintColor = quad.tintColor,
	};
	if (!quads.empty() && quads.back().shader == quad.shader && quads.back().texture == quad.texture) {
		quads.back().instances.push_back(instance);
	} else {
		quads.emplace_back(quad.shader, quad.texture, &memoryResource).instances.push_back(instance);
	}
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

	decltype(TexturedQuadInstances::instances)* instances = nullptr;
	if (!quads.empty() && quads.back().shader == text.shader && quads.back().texture == texture) {
		instances = &quads.back().instances;
	} else {
		instances = &quads.emplace_back(text.shader, texture, &memoryResource).instances;
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
