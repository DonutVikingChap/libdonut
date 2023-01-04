#include <donut/graphics/RenderPass.hpp>

#include <algorithm>                    // std::lower_bound
#include <cstddef>                      // std::size_t
#include <glm/ext/matrix_transform.hpp> // glm::identity, glm::translate, glm::rotate, glm::scale
#include <glm/glm.hpp>                  // glm::...
#include <glm/gtc/matrix_inverse.hpp>   // glm::inverseTranspose
#include <utility>                      // std::move
#include <vector>                       // std::vector, std::erase_if(std::vector)

namespace donut {
namespace graphics {

void RenderPass::reset() noexcept {
	std::erase_if(objectsSortedByShaderAndModel,
		[](const ModelObjectInstancesFromModel& objects) -> bool { return (objects.shader && objects.shader.use_count() <= 1) || objects.model.use_count() <= 1; });
	for (ModelObjectInstancesFromModel& objects : objectsSortedByShaderAndModel) {
		for (std::vector<Model::Object::Instance>& objectInstance : objects.objectInstances) {
			objectInstance.clear();
		}
	}

	transientTextures.clear();

	std::erase_if(quadsSortedByShaderAndTexture,
		[](const TexturedQuadInstancesFromQuad& quads) -> bool { return (quads.shader && quads.shader.use_count() <= 1) || (quads.texture && quads.texture.use_count() <= 1); });
	for (TexturedQuadInstancesFromQuad& quads : quadsSortedByShaderAndTexture) {
		quads.instances.clear();
	}

	std::erase_if(spritesSortedByAtlas, [](const TexturedQuadInstancesFromSprite& sprites) -> bool { return sprites.atlas.use_count() <= 1; });
	for (TexturedQuadInstancesFromSprite& sprites : spritesSortedByAtlas) {
		sprites.instances.clear();
	}

	std::erase_if(glyphsSortedByFont, [](const TexturedQuadInstancesFromText& glyphs) -> bool { return glyphs.font.use_count() <= 1; });
	for (TexturedQuadInstancesFromText& glyphs : glyphsSortedByFont) {
		glyphs.instances.clear();
	}
}

void RenderPass::draw(ModelInstance&& model) {
	assert(model.model);

	const glm::mat4 transformation = glm::scale(model.transformation, {1.0f, -1.0f, 1.0f});
	const glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3{transformation});

	const std::size_t objectCount = model.model->objects.size();

	std::vector<std::vector<Model::Object::Instance>>* objectInstances = nullptr;
	const auto it = std::lower_bound(objectsSortedByShaderAndModel.begin(), objectsSortedByShaderAndModel.end(), model);
	if (it == objectsSortedByShaderAndModel.end()) {
		objectsSortedByShaderAndModel.push_back({.shader = std::move(model.shader), .model = std::move(model.model), .objectInstances{}});
		objectInstances = &objectsSortedByShaderAndModel.back().objectInstances;
		objectInstances->resize(objectCount);
	} else if (it->shader == model.shader && it->model == model.model) {
		objectInstances = &it->objectInstances;
		assert(objectInstances->size() == objectCount);
	} else {
		objectInstances = &objectsSortedByShaderAndModel.insert(it, {.shader = std::move(model.shader), .model = std::move(model.model), .objectInstances{}})->objectInstances;
		objectInstances->resize(objectCount);
	}
	for (std::size_t i = 0; i < objectCount; ++i) {
		(*objectInstances)[i].push_back({
			.transformation = transformation,
			.normalMatrix = normalMatrix,
			.tintColor = model.tintColor,
		});
	}
}

void RenderPass::draw(TransientTextureInstance&& transientTexture) {
	assert(transientTexture.texture);
	glm::mat4 transformation = glm::identity<glm::mat4>();
	transformation = glm::translate(transformation, {transientTexture.position, 0.0f});
	transformation = glm::rotate(transformation, transientTexture.angle, {0.0f, 0.0f, 1.0f});
	transformation = glm::scale(transformation, {transientTexture.texture->getSize() * transientTexture.scale, 1.0f});
	transformation = glm::translate(transformation, {-transientTexture.origin, 0.0f});
	transientTextures.push_back({
		.texture = transientTexture.texture,
		.instance{
			.transformation = transformation,
			.textureOffset = transientTexture.textureOffset,
			.textureScale = transientTexture.textureScale,
			.tintColor = transientTexture.tintColor,
		},
	});
}

void RenderPass::draw(TextureInstance&& texture) {
	assert(texture.texture);
	const glm::vec2 textureSize = texture.texture->getSize(); // NOTE: Copied here since the texture is moved before the initialization of .size below.
	draw(RectangleInstance{
		.texture = std::move(texture.texture),
		.position = texture.position,
		.size = textureSize * texture.scale,
		.angle = texture.angle,
		.origin = texture.origin,
		.textureOffset = texture.textureOffset,
		.textureScale = texture.textureScale,
		.tintColor = texture.tintColor,
	});
}

void RenderPass::draw(RectangleInstance&& rectangle) {
	glm::mat4 transformation = glm::identity<glm::mat4>();
	transformation = glm::translate(transformation, {rectangle.position, 0.0f});
	transformation = glm::rotate(transformation, rectangle.angle, {0.0f, 0.0f, 1.0f});
	transformation = glm::scale(transformation, {rectangle.size, 1.0f});
	transformation = glm::translate(transformation, {-rectangle.origin, 0.0f});
	draw(QuadInstance{
		.texture = std::move(rectangle.texture),
		.transformation = transformation,
		.textureOffset = rectangle.textureOffset,
		.textureScale = rectangle.textureScale,
		.tintColor = rectangle.tintColor,
	});
}

void RenderPass::draw(QuadInstance&& quad) {
	const TexturedQuad::Instance instance{
		.transformation = quad.transformation,
		.textureOffset = quad.textureOffset,
		.textureScale = quad.textureScale,
		.tintColor = quad.tintColor,
	};

	const auto it = std::lower_bound(quadsSortedByShaderAndTexture.begin(), quadsSortedByShaderAndTexture.end(), quad);
	if (it == quadsSortedByShaderAndTexture.end()) {
		quadsSortedByShaderAndTexture.push_back({.shader = std::move(quad.shader), .texture = std::move(quad.texture), .instances{instance}});
	} else if (it->shader == quad.shader && it->texture == quad.texture) {
		it->instances.push_back(instance);
	} else {
		quadsSortedByShaderAndTexture.insert(it, {.shader = std::move(quad.shader), .texture = std::move(quad.texture), .instances{instance}});
	}
}

void RenderPass::draw(SpriteInstance&& sprite) {
	assert(sprite.atlas);
	const SpriteAtlas::Sprite& atlasSprite = sprite.atlas->getSprite(sprite.id);
	glm::mat4 transformation = glm::identity<glm::mat4>();
	transformation = glm::translate(transformation, {sprite.position, 0.0f});
	transformation = glm::rotate(transformation, sprite.angle, {0.0f, 0.0f, 1.0f});
	transformation = glm::scale(transformation, {atlasSprite.size * sprite.scale, 1.0f});
	transformation = glm::translate(transformation, {-sprite.origin, 0.0f});
	const TexturedQuad::Instance instance{
		.transformation = transformation,
		.textureOffset = atlasSprite.textureOffset,
		.textureScale = atlasSprite.textureScale,
		.tintColor = sprite.tintColor,
	};

	const auto it = std::lower_bound(spritesSortedByAtlas.begin(), spritesSortedByAtlas.end(), sprite);
	if (it == spritesSortedByAtlas.end()) {
		spritesSortedByAtlas.push_back({
			.atlas = std::move(sprite.atlas),
			.instances{instance},
		});
	} else if (it->atlas == sprite.atlas) {
		it->instances.push_back(instance);
	} else {
		spritesSortedByAtlas.insert(it, {.atlas = std::move(sprite.atlas), .instances{instance}});
	}
}

void RenderPass::draw(TextInstance&& text) {
	assert(text.font);

	std::vector<TexturedQuad::Instance>* instances = nullptr;
	const auto it = std::lower_bound(glyphsSortedByFont.begin(), glyphsSortedByFont.end(), text);
	if (it == glyphsSortedByFont.end()) {
		glyphsSortedByFont.push_back({.font = std::move(text.font), .instances{}});
		instances = &glyphsSortedByFont.back().instances;
	} else if (it->font == text.font) {
		instances = &it->instances;
	} else {
		instances = &glyphsSortedByFont.insert(it, {.font = std::move(text.font), .instances{}})->instances;
	}
	for (const Font::ShapedText::ShapedGlyph& shapedGlyph : text.text.shapedGlyphs) {
		instances->push_back({
			.transformation = glm::scale(glm::translate(glm::identity<glm::mat4>(), {text.position + shapedGlyph.offset, 0.0f}), {shapedGlyph.size, 1.0f}),
			.textureOffset = shapedGlyph.textureOffset,
			.textureScale = shapedGlyph.textureScale,
			.tintColor = text.color,
		});
	}
}

} // namespace graphics
} // namespace donut
