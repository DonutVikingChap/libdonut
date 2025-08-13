#include <donut/graphics/Font.hpp>
#include <donut/graphics/RenderPass.hpp>
#include <donut/graphics/Text.hpp>
#include <donut/graphics/Texture.hpp>
#include <donut/math.hpp>
#include <donut/unicode.hpp>

#include <algorithm> // std::find
#include <span>      // std::span

namespace donut::graphics {

RenderPass& RenderPass::draw(const ModelInstance& model) {
	assert(model.shader);
	assert(model.model);

	if (previousShader2D || previousShader3D != model.shader) {
		previousShader2D = nullptr;
		previousTexture = nullptr;
		previousSpriteAtlas = nullptr;
		previousFont = nullptr;
		previousShader3D = model.shader;
		commandBuffer.push_back(CommandUseShader3D{.shader = model.shader});
	}

	if (previousModel != model.model || previousDiffuseMapOverride != model.diffuseMapOverride || previousSpecularMapOverride != model.specularMapOverride ||
		previousNormalMapOverride != model.normalMapOverride || previousEmissiveMapOverride != model.emissiveMapOverride) {
		previousModel = model.model;
		previousDiffuseMapOverride = model.diffuseMapOverride;
		previousSpecularMapOverride = model.specularMapOverride;
		previousNormalMapOverride = model.normalMapOverride;
		previousEmissiveMapOverride = model.emissiveMapOverride;
		commandBuffer.push_back(CommandUseModel{
			.model = model.model,
			.diffuseMapOverride = model.diffuseMapOverride,
			.specularMapOverride = model.specularMapOverride,
			.normalMapOverride = model.normalMapOverride,
			.emissiveMapOverride = model.emissiveMapOverride,
		});
	}

	commandBuffer.push_back(CommandDrawModelInstance{
		.transformation = model.transformation,
		.tintColor = model.tintColor,
		.textureOffset = model.textureOffset,
		.textureScale = model.textureScale,
		.specularFactor = model.specularFactor,
		.emissiveFactor = model.emissiveFactor,
	});
	return *this;
}

RenderPass& RenderPass::draw(const QuadInstance& quad) {
	assert(quad.shader);
	assert(quad.texture);

	if (previousShader3D || previousShader2D != quad.shader) {
		previousShader3D = nullptr;
		previousModel = nullptr;
		previousDiffuseMapOverride = nullptr;
		previousSpecularMapOverride = nullptr;
		previousNormalMapOverride = nullptr;
		previousEmissiveMapOverride = nullptr;
		previousShader2D = quad.shader;
		commandBuffer.push_back(CommandUseShader2D{.shader = quad.shader});
	}

	if (previousTexture != quad.texture) {
		previousTexture = quad.texture;
		commandBuffer.push_back(CommandUseTexture{.texture = quad.texture});
	}

	commandBuffer.push_back(CommandDrawQuadInstance{
		.transformation = quad.transformation,
		.tintColor = quad.tintColor,
		.textureOffset = quad.textureOffset,
		.textureScale = quad.textureScale,
	});
	return *this;
}

RenderPass& RenderPass::draw(const TextureInstance& texture) {
	assert(texture.shader);
	assert(texture.texture);

	if (previousShader3D || previousShader2D != texture.shader) {
		previousShader3D = nullptr;
		previousModel = nullptr;
		previousDiffuseMapOverride = nullptr;
		previousSpecularMapOverride = nullptr;
		previousNormalMapOverride = nullptr;
		previousEmissiveMapOverride = nullptr;
		previousShader2D = texture.shader;
		commandBuffer.push_back(CommandUseShader2D{.shader = texture.shader});
	}

	if (previousTexture != texture.texture) {
		previousTexture = texture.texture;
		commandBuffer.push_back(CommandUseTexture{.texture = texture.texture});
	}

	commandBuffer.push_back(CommandDrawTextureInstance{
		.tintColor = texture.tintColor,
		.position = texture.position,
		.scale = texture.scale,
		.origin = texture.origin,
		.textureOffset = texture.textureOffset,
		.textureScale = texture.textureScale,
		.angle = texture.angle,
	});
	return *this;
}

RenderPass& RenderPass::draw(const RectangleInstance& rectangle) {
	assert(rectangle.shader);
	assert(rectangle.texture);

	if (previousShader3D || previousShader2D != rectangle.shader) {
		previousShader3D = nullptr;
		previousModel = nullptr;
		previousDiffuseMapOverride = nullptr;
		previousSpecularMapOverride = nullptr;
		previousNormalMapOverride = nullptr;
		previousEmissiveMapOverride = nullptr;
		previousShader2D = rectangle.shader;
		commandBuffer.push_back(CommandUseShader2D{.shader = rectangle.shader});
	}

	if (previousTexture != rectangle.texture) {
		previousTexture = rectangle.texture;
		commandBuffer.push_back(CommandUseTexture{.texture = rectangle.texture});
	}

	commandBuffer.push_back(CommandDrawRectangleInstance{
		.tintColor = rectangle.tintColor,
		.position = rectangle.position,
		.size = rectangle.size,
		.origin = rectangle.origin,
		.textureOffset = rectangle.textureOffset,
		.textureScale = rectangle.textureScale,
		.angle = rectangle.angle,
	});
	return *this;
}

RenderPass& RenderPass::draw(const SpriteInstance& sprite) {
	assert(sprite.shader);
	assert(sprite.atlas);

	if (previousShader3D || previousShader2D != sprite.shader) {
		previousShader3D = nullptr;
		previousModel = nullptr;
		previousDiffuseMapOverride = nullptr;
		previousSpecularMapOverride = nullptr;
		previousNormalMapOverride = nullptr;
		previousEmissiveMapOverride = nullptr;
		previousShader2D = sprite.shader;
		commandBuffer.push_back(CommandUseShader2D{.shader = sprite.shader});
	}

	const Texture* const texture = &sprite.atlas->getAtlasTexture();

	if (previousTexture != texture || previousSpriteAtlas != sprite.atlas) {
		previousTexture = texture;
		previousSpriteAtlas = sprite.atlas;
		commandBuffer.push_back(CommandUseSpriteAtlas{.atlas = sprite.atlas});
	}

	commandBuffer.push_back(CommandDrawSpriteInstance{
		.tintColor = sprite.tintColor,
		.position = sprite.position,
		.scale = sprite.scale,
		.origin = sprite.origin,
		.angle = sprite.angle,
		.id = sprite.id,
	});
	return *this;
}

RenderPass& RenderPass::draw(const TextInstance& text) {
	assert(text.shader);
	assert(text.text);

	if (previousShader3D || previousShader2D != text.shader) {
		previousShader3D = nullptr;
		previousModel = nullptr;
		previousDiffuseMapOverride = nullptr;
		previousSpecularMapOverride = nullptr;
		previousNormalMapOverride = nullptr;
		previousEmissiveMapOverride = nullptr;
		previousShader2D = text.shader;
		commandBuffer.push_back(CommandUseShader2D{.shader = text.shader});
	}

	for (const Text::ShapedGlyph& shapedGlyph : text.text->getShapedGlyphs()) {
		assert(shapedGlyph.font);
		shapedGlyph.font->markGlyphForRendering(shapedGlyph.characterSize, shapedGlyph.codePoint);
		if (shapedGlyph.font->containsGlyphsMarkedForRendering()) {
			[[unlikely]];
			if (std::find(fonts.begin(), fonts.end(), shapedGlyph.font) == fonts.end()) {
				fonts.push_back(shapedGlyph.font);
			}
		}
		const Texture* const texture = &shapedGlyph.font->getAtlasTexture();
		if (previousTexture != texture || previousFont != shapedGlyph.font) {
			previousFont = shapedGlyph.font;
			previousTexture = texture;
		}
	}

	commandBuffer.push_back(CommandDrawTextInstance{
		.color = text.color,
		.text = text.text,
		.position = text.position,
	});
	return *this;
}

RenderPass& RenderPass::draw(const TextCopyInstance& text) {
	assert(text.shader);
	assert(text.text);

	if (previousShader3D || previousShader2D != text.shader) {
		previousShader3D = nullptr;
		previousModel = nullptr;
		previousDiffuseMapOverride = nullptr;
		previousSpecularMapOverride = nullptr;
		previousNormalMapOverride = nullptr;
		previousEmissiveMapOverride = nullptr;
		previousShader2D = text.shader;
		commandBuffer.push_back(CommandUseShader2D{.shader = text.shader});
	}

	for (const Text::ShapedGlyph& shapedGlyph : text.text->getShapedGlyphs()) {
		assert(shapedGlyph.font);
		shapedGlyph.font->markGlyphForRendering(shapedGlyph.characterSize, shapedGlyph.codePoint);
		if (shapedGlyph.font->containsGlyphsMarkedForRendering()) {
			[[unlikely]];
			if (std::find(fonts.begin(), fonts.end(), shapedGlyph.font) == fonts.end()) {
				fonts.push_back(shapedGlyph.font);
			}
		}
		const Texture* const texture = &shapedGlyph.font->getAtlasTexture();
		if (previousTexture != texture || previousFont != shapedGlyph.font) {
			previousFont = shapedGlyph.font;
			previousTexture = texture;
		}
	}

	const std::span<const Text::ShapedGlyph> shapedGlyphsData = commandBuffer.append(std::span<const Text::ShapedGlyph>{text.text->getShapedGlyphs()});
	commandBuffer.push_back(CommandDrawTextCopyInstance{
		.color = text.color,
		.shapedGlyphs = shapedGlyphsData,
		.position = text.position,
	});
	return *this;
}

RenderPass& RenderPass::draw(const TextUTF8StringInstance& text) {
	assert(text.shader);
	assert(text.font);

	if (previousShader3D || previousShader2D != text.shader) {
		previousShader3D = nullptr;
		previousModel = nullptr;
		previousDiffuseMapOverride = nullptr;
		previousSpecularMapOverride = nullptr;
		previousNormalMapOverride = nullptr;
		previousEmissiveMapOverride = nullptr;
		previousShader2D = text.shader;
		commandBuffer.push_back(CommandUseShader2D{.shader = text.shader});
	}

	for (const char32_t codePoint : unicode::UTF8View{text.string}) {
		text.font->markGlyphForRendering(text.characterSize, codePoint);
	}
	if (text.font->containsGlyphsMarkedForRendering()) {
		[[unlikely]];
		if (std::find(fonts.begin(), fonts.end(), text.font) == fonts.end()) {
			fonts.push_back(text.font);
		}
	}
	const Texture* const texture = &text.font->getAtlasTexture();
	if (previousTexture != texture || previousFont != text.font) {
		previousTexture = texture;
		previousFont = text.font;
		commandBuffer.push_back(CommandUseFont{.font = text.font});
	}

	static_assert(sizeof(char) == sizeof(char8_t));
	static_assert(alignof(char) == alignof(char8_t));
	const std::span<const char> stringData = commandBuffer.append(std::span<const char>{reinterpret_cast<const char*>(text.string.data()), text.string.size()});
	commandBuffer.push_back(CommandDrawTextStringInstance{
		.color = text.color,
		.string{stringData.data(), stringData.size()},
		.position = text.position,
		.scale = text.scale,
		.origin = text.origin,
		.characterSize = text.characterSize,
	});
	return *this;
}

RenderPass& RenderPass::draw(const TextStringInstance& text) {
	assert(text.shader);
	assert(text.font);

	if (previousShader3D || previousShader2D != text.shader) {
		previousShader3D = nullptr;
		previousModel = nullptr;
		previousDiffuseMapOverride = nullptr;
		previousSpecularMapOverride = nullptr;
		previousNormalMapOverride = nullptr;
		previousEmissiveMapOverride = nullptr;
		previousShader2D = text.shader;
		commandBuffer.push_back(CommandUseShader2D{.shader = text.shader});
	}

	for (const char32_t codePoint : unicode::UTF8View{text.string}) {
		text.font->markGlyphForRendering(text.characterSize, codePoint);
	}
	if (text.font->containsGlyphsMarkedForRendering()) {
		[[unlikely]];
		if (std::find(fonts.begin(), fonts.end(), text.font) == fonts.end()) {
			fonts.push_back(text.font);
		}
	}
	const Texture* const texture = &text.font->getAtlasTexture();
	if (previousTexture != texture || previousFont != text.font) {
		previousTexture = texture;
		previousFont = text.font;
		commandBuffer.push_back(CommandUseFont{.font = text.font});
	}

	const std::span<const char> stringData = commandBuffer.append(std::span<const char>{text.string.data(), text.string.size()});
	commandBuffer.push_back(CommandDrawTextStringInstance{
		.color = text.color,
		.string{stringData.data(), stringData.size()},
		.position = text.position,
		.scale = text.scale,
		.origin = text.origin,
		.characterSize = text.characterSize,
	});
	return *this;
}

} // namespace donut::graphics
