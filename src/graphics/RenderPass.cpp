#include <donut/graphics/RenderPass.hpp>

#include <algorithm>
#include <cstddef>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <utility>
#include <vector>

namespace donut {
namespace graphics {

void RenderPass::draw(Model&& model) {
	assert(model.scene);

	const glm::mat4 transformation = glm::scale(model.transformation, {1.0f, -1.0f, 1.0f});
	const glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3{transformation});

	const std::size_t objectCount = model.scene->objects.size();

	std::vector<std::vector<Scene::Object::Instance>>* objectInstances = nullptr;
	const auto it = std::lower_bound(modelInstancesSortedByShaderAndScene.begin(), modelInstancesSortedByShaderAndScene.end(), model);
	if (it == modelInstancesSortedByShaderAndScene.end()) {
		modelInstancesSortedByShaderAndScene.push_back({.shader = std::move(model.shader), .scene = std::move(model.scene), .objectInstances{}});
		objectInstances = &modelInstancesSortedByShaderAndScene.back().objectInstances;
		objectInstances->resize(objectCount);
	} else if (it->shader == model.shader && it->scene == model.scene) {
		objectInstances = &it->objectInstances;
		assert(objectInstances->size() == objectCount);
	} else {
		objectInstances =
			&modelInstancesSortedByShaderAndScene.insert(it, {.shader = std::move(model.shader), .scene = std::move(model.scene), .objectInstances{}})->objectInstances;
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

void RenderPass::draw(TransientTexture&& transientTexture) {
	assert(transientTexture.texture);
	glm::mat4 transformation = glm::identity<glm::mat4>();
	transformation = glm::translate(transformation, {transientTexture.position, 0.0f});
	transformation = glm::rotate(transformation, transientTexture.angle, {0.0f, 0.0f, 1.0f});
	transformation = glm::scale(transformation, {transientTexture.texture->getSize() * transientTexture.scale, 1.0f});
	transformation = glm::translate(transformation, {-transientTexture.origin, 0.0f});
	transientTextureInstances.push_back({
		.texture = transientTexture.texture,
		.instance{
			.transformation = transformation,
			.textureOffset = transientTexture.textureOffset,
			.textureScale = transientTexture.textureScale,
			.tintColor = transientTexture.tintColor,
		},
	});
}

void RenderPass::draw(Quad&& quad) {
	const TexturedQuad::Instance instance{
		.transformation = quad.transformation,
		.textureOffset = quad.textureOffset,
		.textureScale = quad.textureScale,
		.tintColor = quad.tintColor,
	};

	const auto it = std::lower_bound(quadInstancesSortedByShaderAndTexture.begin(), quadInstancesSortedByShaderAndTexture.end(), quad);
	if (it == quadInstancesSortedByShaderAndTexture.end()) {
		quadInstancesSortedByShaderAndTexture.push_back({.shader = std::move(quad.shader), .texture = std::move(quad.texture), .instances{instance}});
	} else if (it->shader == quad.shader && it->texture == quad.texture) {
		it->instances.push_back(instance);
	} else {
		quadInstancesSortedByShaderAndTexture.insert(it, {.shader = std::move(quad.shader), .texture = std::move(quad.texture), .instances{instance}});
	}
}

void RenderPass::draw(Sprite&& sprite) {
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

	const auto it = std::lower_bound(spriteInstancesSortedByAtlas.begin(), spriteInstancesSortedByAtlas.end(), sprite);
	if (it == spriteInstancesSortedByAtlas.end()) {
		spriteInstancesSortedByAtlas.push_back({
			.atlas = std::move(sprite.atlas),
			.instances{instance},
		});
	} else if (it->atlas == sprite.atlas) {
		it->instances.push_back(instance);
	} else {
		spriteInstancesSortedByAtlas.insert(it, {.atlas = std::move(sprite.atlas), .instances{instance}});
	}
}

void RenderPass::draw(Text&& text) {
	assert(text.font);

	std::vector<TexturedQuad::Instance>* instances = nullptr;
	const auto it = std::lower_bound(glyphInstancesSortedByFont.begin(), glyphInstancesSortedByFont.end(), text);
	if (it == glyphInstancesSortedByFont.end()) {
		glyphInstancesSortedByFont.push_back({.font = std::move(text.font), .instances{}});
		instances = &glyphInstancesSortedByFont.back().instances;
	} else if (it->font == text.font) {
		instances = &it->instances;
	} else {
		instances = &glyphInstancesSortedByFont.insert(it, {.font = std::move(text.font), .instances{}})->instances;
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

void RenderPass::clear() noexcept {
	backgroundColor.reset();

	std::erase_if(modelInstancesSortedByShaderAndScene, [](const ModelInstances& models) -> bool { return models.shader.use_count() <= 1 || models.scene.use_count() <= 1; });
	for (ModelInstances& models : modelInstancesSortedByShaderAndScene) {
		for (std::vector<Scene::Object::Instance>& objectInstance : models.objectInstances) {
			objectInstance.clear();
		}
	}

	transientTextureInstances.clear();

	std::erase_if(quadInstancesSortedByShaderAndTexture, [](const QuadInstances& quads) -> bool { return quads.shader.use_count() <= 1 || quads.texture.use_count() <= 1; });
	for (QuadInstances& quads : quadInstancesSortedByShaderAndTexture) {
		quads.instances.clear();
	}

	std::erase_if(spriteInstancesSortedByAtlas, [](const SpriteInstances& sprites) -> bool { return sprites.atlas.use_count() <= 1; });
	for (SpriteInstances& sprites : spriteInstancesSortedByAtlas) {
		sprites.instances.clear();
	}

	std::erase_if(glyphInstancesSortedByFont, [](const GlyphInstances& glyphs) -> bool { return glyphs.font.use_count() <= 1; });
	for (GlyphInstances& glyphs : glyphInstancesSortedByFont) {
		glyphs.instances.clear();
	}
}

} // namespace graphics
} // namespace donut
