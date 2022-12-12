#ifndef DONUT_GRAPHICS_RENDER_PASS_HPP
#define DONUT_GRAPHICS_RENDER_PASS_HPP

#include <donut/Color.hpp>
#include <donut/graphics/Font.hpp>
#include <donut/graphics/Scene.hpp>
#include <donut/graphics/Shader2D.hpp>
#include <donut/graphics/Shader3D.hpp>
#include <donut/graphics/SpriteAtlas.hpp>
#include <donut/graphics/Texture.hpp>
#include <donut/graphics/TexturedQuad.hpp>

#include <cassert>                      // assert
#include <compare>                      // std::strong_ordering
#include <glm/ext/matrix_transform.hpp> // glm::identity, glm::translate, glm::rotate, glm::scale
#include <glm/glm.hpp>                  // glm::...
#include <memory>                       // std::shared_ptr
#include <optional>                     // std::optional
#include <utility>                      // std::move
#include <vector>                       // std::vector

namespace donut {
namespace graphics {

struct Model {
	std::shared_ptr<Shader3D> shader{};
	std::shared_ptr<Scene> scene;
	glm::mat4 transformation = glm::identity<glm::mat4>();
	Color tintColor = Color::WHITE;
};

struct TransientTexture {
	const Texture* texture;
	glm::vec2 position{0.0f, 0.0f};
	glm::vec2 scale{1.0f, 1.0f};
	float angle = 0.0f;
	glm::vec2 origin{0.0f, 0.0f};
	glm::vec2 textureOffset{0.0f, 0.0f};
	glm::vec2 textureScale{1.0f, 1.0f};
	Color tintColor = Color::WHITE;
};

struct Rectangle {
	std::shared_ptr<Texture> texture{};
	glm::vec2 position{0.0f, 0.0f};
	glm::vec2 size{1.0f, 1.0f};
	float angle = 0.0f;
	glm::vec2 origin{0.0f, 0.0f};
	glm::vec2 textureOffset{0.0f, 0.0f};
	glm::vec2 textureScale{1.0f, 1.0f};
	Color tintColor = Color::WHITE;
};

struct ImageSizedRectangle {
	std::shared_ptr<Texture> texture;
	glm::vec2 position{0.0f, 0.0f};
	glm::vec2 scale{1.0f, 1.0f};
	float angle = 0.0f;
	glm::vec2 origin{0.0f, 0.0f};
	glm::vec2 textureOffset{0.0f, 0.0f};
	glm::vec2 textureScale{1.0f, 1.0f};
	Color tintColor = Color::WHITE;
};

struct Quad {
	std::shared_ptr<Shader2D> shader{};
	std::shared_ptr<Texture> texture{};
	glm::mat4 transformation = glm::identity<glm::mat4>();
	glm::vec2 textureOffset{0.0f, 0.0f};
	glm::vec2 textureScale{1.0f, 1.0f};
	Color tintColor = Color::WHITE;
};

struct Sprite {
	std::shared_ptr<SpriteAtlas> atlas;
	SpriteAtlas::SpriteId id;
	glm::vec2 position{0.0f, 0.0f};
	glm::vec2 scale{1.0f, 1.0f};
	float angle = 0.0f;
	glm::vec2 origin{0.0f, 0.0f};
	Color tintColor = Color::WHITE;
};

struct Text {
	std::shared_ptr<Font> font;
	Font::ShapedText text;
	glm::vec2 position;
	Color color = Color::WHITE;
};

class RenderPass {
public:
	void setBackgroundColor(std::optional<Color> color) {
		backgroundColor = color;
	}

	void draw(Model&& model);
	void draw(TransientTexture&& transientTexture);
	void draw(Quad&& quad);
	void draw(Sprite&& sprite);
	void draw(Text&& text);

	void draw(Rectangle&& rectangle) {
		glm::mat4 transformation = glm::identity<glm::mat4>();
		transformation = glm::translate(transformation, {rectangle.position, 0.0f});
		transformation = glm::rotate(transformation, rectangle.angle, {0.0f, 0.0f, 1.0f});
		transformation = glm::scale(transformation, {rectangle.size, 1.0f});
		transformation = glm::translate(transformation, {-rectangle.origin, 0.0f});
		draw(Quad{
			.texture = std::move(rectangle.texture),
			.transformation = transformation,
			.textureOffset = rectangle.textureOffset,
			.textureScale = rectangle.textureScale,
			.tintColor = rectangle.tintColor,
		});
	}

	void draw(ImageSizedRectangle&& rectangle) {
		assert(rectangle.texture);
		const glm::vec2 textureSize = rectangle.texture->getSize();
		draw(Rectangle{
			.texture = std::move(rectangle.texture),
			.position = rectangle.position,
			.size = textureSize * rectangle.scale,
			.angle = rectangle.angle,
			.origin = rectangle.origin,
			.textureOffset = rectangle.textureOffset,
			.textureScale = rectangle.textureScale,
			.tintColor = rectangle.tintColor,
		});
	}

	void reset() noexcept;

private:
	friend Renderer;

	template <typename Shader>
	[[nodiscard]] static std::strong_ordering compareShaders(const std::shared_ptr<Shader>& a, const std::shared_ptr<Shader>& b) noexcept {
		const int orderIndexA = (a) ? a->options.orderIndex : 0;
		const int orderIndexB = (b) ? b->options.orderIndex : 0;
		return orderIndexA <=> orderIndexB;
	}

	struct ModelInstances {
		std::shared_ptr<Shader3D> shader;
		std::shared_ptr<Scene> scene;
		std::vector<std::vector<Scene::Object::Instance>> objectInstances;

		[[nodiscard]] std::strong_ordering operator<=>(const ModelInstances& other) const {
			const std::strong_ordering shaderOrdering = compareShaders(shader, other.shader);
			return (shaderOrdering != std::strong_ordering::equal) ? shaderOrdering : scene.get() <=> other.scene.get();
		}

		[[nodiscard]] std::strong_ordering operator<=>(const Model& model) const {
			const std::strong_ordering shaderOrdering = compareShaders(shader, model.shader);
			return (shaderOrdering != std::strong_ordering::equal) ? shaderOrdering : scene.get() <=> model.scene.get();
		}
	};

	struct TransientTextureInstance {
		const Texture* texture;
		TexturedQuad::Instance instance;
	};

	struct QuadInstances {
		std::shared_ptr<Shader2D> shader;
		std::shared_ptr<Texture> texture;
		std::vector<TexturedQuad::Instance> instances;

		[[nodiscard]] std::strong_ordering operator<=>(const QuadInstances& other) const {
			const std::strong_ordering shaderOrdering = compareShaders(shader, other.shader);
			return (shaderOrdering != std::strong_ordering::equal) ? shaderOrdering : texture.get() <=> other.texture.get();
		}

		[[nodiscard]] std::strong_ordering operator<=>(const Quad& quad) const {
			const std::strong_ordering shaderOrdering = compareShaders(shader, quad.shader);
			return (shaderOrdering != std::strong_ordering::equal) ? shaderOrdering : texture.get() <=> quad.texture.get();
		}
	};

	struct SpriteInstances {
		std::shared_ptr<SpriteAtlas> atlas;
		std::vector<TexturedQuad::Instance> instances;

		[[nodiscard]] std::strong_ordering operator<=>(const SpriteInstances& other) const {
			return atlas.get() <=> other.atlas.get();
		}

		[[nodiscard]] std::strong_ordering operator<=>(const Sprite& sprite) const {
			return atlas.get() <=> sprite.atlas.get();
		}
	};

	struct GlyphInstances {
		std::shared_ptr<Font> font;
		std::vector<TexturedQuad::Instance> instances;

		[[nodiscard]] std::strong_ordering operator<=>(const GlyphInstances& other) const {
			return font.get() <=> other.font.get();
		}

		[[nodiscard]] std::strong_ordering operator<=>(const Text& text) const {
			return font.get() <=> text.font.get();
		}
	};

	std::optional<Color> backgroundColor{};
	std::vector<ModelInstances> modelInstancesSortedByShaderAndScene{};
	std::vector<TransientTextureInstance> transientTextureInstances{};
	std::vector<QuadInstances> quadInstancesSortedByShaderAndTexture{};
	std::vector<SpriteInstances> spriteInstancesSortedByAtlas{};
	std::vector<GlyphInstances> glyphInstancesSortedByFont{};
};

} // namespace graphics
} // namespace donut

#endif
