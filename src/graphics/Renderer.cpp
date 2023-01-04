#include <donut/Variant.hpp>
#include <donut/graphics/Font.hpp>
#include <donut/graphics/Framebuffer.hpp>
#include <donut/graphics/Model.hpp>
#include <donut/graphics/Renderer.hpp>
#include <donut/graphics/SpriteAtlas.hpp>
#include <donut/graphics/Texture.hpp>
#include <donut/graphics/TexturedQuad.hpp>
#include <donut/graphics/Viewport.hpp>
#include <donut/graphics/opengl.hpp>

#include <cassert>              // assert
#include <cstddef>              // std::size_t
#include <glm/glm.hpp>          // glm::...
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr
#include <span>                 // std::span
#include <vector>               // std::vector

namespace donut {
namespace graphics {

namespace {

void useShader(Shader3D& shader, const glm::mat4& projectionViewMatrix) {
	glUseProgram(shader.program.get());

	for (const auto& [location, value] : shader.program.getUniformUploadQueue()) {
		match(value)([location = location](float v) -> void { glUniform1f(location, v); },
			[location = location](glm::vec2 v) -> void { glUniform2f(location, v.x, v.y); },
			[location = location](glm::vec3 v) -> void { glUniform3f(location, v.x, v.y, v.z); },
			[location = location](glm::vec4 v) -> void { glUniform4f(location, v.x, v.y, v.z, v.w); },
			[location = location](glm::i32 v) -> void { glUniform1i(location, v); },
			[location = location](glm::i32vec2 v) -> void { glUniform2i(location, v.x, v.y); },
			[location = location](glm::i32vec3 v) -> void { glUniform3i(location, v.x, v.y, v.z); },
			[location = location](glm::i32vec4 v) -> void { glUniform4i(location, v.x, v.y, v.z, v.w); },
			[location = location](glm::u32 v) -> void { glUniform1ui(location, v); },
			[location = location](glm::u32vec2 v) -> void { glUniform2ui(location, v.x, v.y); },
			[location = location](glm::u32vec3 v) -> void { glUniform3ui(location, v.x, v.y, v.z); },
			[location = location](glm::u32vec4 v) -> void { glUniform4ui(location, v.x, v.y, v.z, v.w); },
			[location = location](const glm::mat2& v) -> void { glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(v)); },
			[location = location](const glm::mat3& v) -> void { glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(v)); },
			[location = location](const glm::mat4& v) -> void { glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(v)); });
	}
	shader.program.clearUniformUploadQueue();

	glUniformMatrix4fv(shader.projectionViewMatrix.getLocation(), 1, GL_FALSE, glm::value_ptr(projectionViewMatrix));
	glUniform1i(shader.diffuseMap.getLocation(), Model::Object::TEXTURE_UNIT_DIFFUSE);
	glUniform1i(shader.specularMap.getLocation(), Model::Object::TEXTURE_UNIT_SPECULAR);
	glUniform1i(shader.normalMap.getLocation(), Model::Object::TEXTURE_UNIT_NORMAL);

	if (shader.options.overwriteDepthBuffer) {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);
	} else if (shader.options.useDepthTest) {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
	} else {
		glDisable(GL_DEPTH_TEST);
	}

	glDisable(GL_STENCIL_TEST);

	if (shader.options.useBackfaceCulling) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
	} else {
		glDisable(GL_CULL_FACE);
	}

	if (shader.options.useAlphaBlending) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else {
		glDisable(GL_BLEND);
	}
}

void useShader(Shader2D& shader, const glm::mat4& projectionViewMatrix) {
	glUseProgram(shader.program.get());

	for (const auto& [location, value] : shader.program.getUniformUploadQueue()) {
		match(value)([location = location](float v) -> void { glUniform1f(location, v); },
			[location = location](glm::vec2 v) -> void { glUniform2f(location, v.x, v.y); },
			[location = location](glm::vec3 v) -> void { glUniform3f(location, v.x, v.y, v.z); },
			[location = location](glm::vec4 v) -> void { glUniform4f(location, v.x, v.y, v.z, v.w); },
			[location = location](glm::i32 v) -> void { glUniform1i(location, v); },
			[location = location](glm::i32vec2 v) -> void { glUniform2i(location, v.x, v.y); },
			[location = location](glm::i32vec3 v) -> void { glUniform3i(location, v.x, v.y, v.z); },
			[location = location](glm::i32vec4 v) -> void { glUniform4i(location, v.x, v.y, v.z, v.w); },
			[location = location](glm::u32 v) -> void { glUniform1ui(location, v); },
			[location = location](glm::u32vec2 v) -> void { glUniform2ui(location, v.x, v.y); },
			[location = location](glm::u32vec3 v) -> void { glUniform3ui(location, v.x, v.y, v.z); },
			[location = location](glm::u32vec4 v) -> void { glUniform4ui(location, v.x, v.y, v.z, v.w); },
			[location = location](const glm::mat2& v) -> void { glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(v)); },
			[location = location](const glm::mat3& v) -> void { glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(v)); },
			[location = location](const glm::mat4& v) -> void { glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(v)); });
	}
	shader.program.clearUniformUploadQueue();

	glUniformMatrix4fv(shader.projectionViewMatrix.getLocation(), 1, GL_FALSE, glm::value_ptr(projectionViewMatrix));
	glUniform1i(shader.textureUnit.getLocation(), TexturedQuad::TEXTURE_UNIT);

	if (shader.options.overwriteDepthBuffer) {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);
	} else if (shader.options.useDepthTest) {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
	} else {
		glDisable(GL_DEPTH_TEST);
	}

	glDisable(GL_STENCIL_TEST);

	if (shader.options.useBackfaceCulling) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
	} else {
		glDisable(GL_CULL_FACE);
	}

	if (shader.options.useAlphaBlending) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else {
		glDisable(GL_BLEND);
	}
}

void renderInstances(Shader3D&, const Model::Object& object, std::span<const Model::Object::Instance> instances) {
	glBufferData(
		GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(instances.size() * sizeof(Model::Object::Instance)), instances.data(), static_cast<GLenum>(Model::Object::INSTANCES_USAGE));
	glDrawElementsInstanced(static_cast<GLenum>(Model::Object::PRIMITIVE_TYPE),
		static_cast<GLsizei>(object.indexCount),
		static_cast<GLenum>(Model::Object::INDEX_TYPE),
		nullptr,
		static_cast<GLsizei>(instances.size()));
}

void renderInstances(Shader3D& shader, std::span<const Model::Object> objects, std::span<const std::vector<Model::Object::Instance>> objectInstances, const Texture& whiteTexture,
	const Texture& grayTexture, const Texture& normalTexture) {
	assert(objects.size() == objectInstances.size());
	for (std::size_t i = 0; i < objects.size(); ++i) {
		const Model::Object& object = objects[i];

		glBindVertexArray(object.mesh.get());
		glBindBuffer(GL_ARRAY_BUFFER, object.mesh.getInstanceBuffer());

		glActiveTexture(GL_TEXTURE0 + Model::Object::TEXTURE_UNIT_DIFFUSE);
		glBindTexture(GL_TEXTURE_2D, (object.material.diffuseMap) ? object.material.diffuseMap.get() : whiteTexture.get());

		glActiveTexture(GL_TEXTURE0 + Model::Object::TEXTURE_UNIT_SPECULAR);
		glBindTexture(GL_TEXTURE_2D, (object.material.specularMap) ? object.material.specularMap.get() : grayTexture.get());

		glActiveTexture(GL_TEXTURE0 + Model::Object::TEXTURE_UNIT_NORMAL);
		glBindTexture(GL_TEXTURE_2D, (object.material.normalMap) ? object.material.normalMap.get() : normalTexture.get());

		glUniform1f(shader.specularExponent.getLocation(), object.material.specularExponent);

		renderInstances(shader, object, objectInstances[i]);
	}
}

void renderInstances(Shader2D&, const TexturedQuad&, std::span<const TexturedQuad::Instance> instances) {
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(instances.size() * sizeof(TexturedQuad::Instance)), instances.data(), static_cast<GLenum>(TexturedQuad::INSTANCES_USAGE));
	glDrawArraysInstanced(static_cast<GLenum>(TexturedQuad::PRIMITIVE_TYPE), 0, static_cast<GLsizei>(TexturedQuad::VERTICES.size()), static_cast<GLsizei>(instances.size()));
}

} // namespace

void Renderer::clearFramebufferDepth(Framebuffer& framebuffer) {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.get());
	glClear(GL_DEPTH_BUFFER_BIT);
}

void Renderer::clearFramebufferStencil(Framebuffer& framebuffer, std::int32_t stencilValue) {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.get());
	glClearStencil(static_cast<GLint>(stencilValue));
	glClear(GL_STENCIL_BUFFER_BIT);
}

void Renderer::clearFramebufferDepthAndStencil(Framebuffer& framebuffer, std::int32_t stencilValue) {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.get());
	glClearStencil(static_cast<GLint>(stencilValue));
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer::clearFramebufferColor(Framebuffer& framebuffer, Color color) {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.get());
	glClearColor(color.getRedComponent(), color.getGreenComponent(), color.getBlueComponent(), color.getAlphaComponent());
	glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::clearFramebufferColorAndDepth(Framebuffer& framebuffer, Color color) {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.get());
	glClearColor(color.getRedComponent(), color.getGreenComponent(), color.getBlueComponent(), color.getAlphaComponent());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::clearFramebufferColorAndStencil(Framebuffer& framebuffer, Color color, std::int32_t stencilValue) {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.get());
	glClearColor(color.getRedComponent(), color.getGreenComponent(), color.getBlueComponent(), color.getAlphaComponent());
	glClearStencil(static_cast<GLint>(stencilValue));
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer::clearFramebufferColorAndDepthAndStencil(Framebuffer& framebuffer, Color color, std::int32_t stencilValue) {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.get());
	glClearColor(color.getRedComponent(), color.getGreenComponent(), color.getBlueComponent(), color.getAlphaComponent());
	glClearStencil(static_cast<GLint>(stencilValue));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer::render(Framebuffer& framebuffer, const RenderPass& renderPass, const Viewport& viewport, const glm::mat4& projectionViewMatrix) {
	// Bind framebuffer.
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.get());

	// Setup viewport.
	glViewport(viewport.position.x, viewport.position.y, viewport.size.x, viewport.size.y);

	// Render 3D objects.
	{
		// Render models.
		if (!renderPass.objectsSortedByShaderAndModel.empty()) {
			const RenderPass::ModelObjectInstancesFromModel& firstModels = renderPass.objectsSortedByShaderAndModel.front();

			Shader3D* shader = firstModels.shader.get();
			Shader3D* actualShader = (shader) ? shader : &modelShader;
			useShader(*actualShader, projectionViewMatrix);

			const Model* model = firstModels.model.get();
			renderInstances(*actualShader, model->objects, firstModels.objectInstances, whiteTexture, grayTexture, normalTexture);

			for (const RenderPass::ModelObjectInstancesFromModel& models : std::span{renderPass.objectsSortedByShaderAndModel}.subspan(1)) {
				if (shader != models.shader.get()) {
					shader = models.shader.get();
					actualShader = (shader) ? shader : &modelShader;
					useShader(*actualShader, projectionViewMatrix);
				}

				renderInstances(*actualShader, models.model->objects, models.objectInstances, whiteTexture, grayTexture, normalTexture);
			}
		}
	}

	// Render 2D textured quads.
	{
		glBindVertexArray(texturedQuad.mesh.get());
		glBindBuffer(GL_ARRAY_BUFFER, texturedQuad.mesh.getInstanceBuffer());

		glActiveTexture(GL_TEXTURE0 + TexturedQuad::TEXTURE_UNIT);

		// Render transient textures.
		if (!renderPass.transientTextures.empty()) {
			const RenderPass::TexturedQuadInstanceFromTransientTexture& firstTransientTexture = renderPass.transientTextures.front();

			useShader(defaultShader, projectionViewMatrix);

			glBindTexture(GL_TEXTURE_2D, firstTransientTexture.texture->get());

			renderInstances(defaultShader, texturedQuad, std::span{&firstTransientTexture.instance, 1});

			for (const RenderPass::TexturedQuadInstanceFromTransientTexture& transientTexture : std::span{renderPass.transientTextures}.subspan(1)) {
				glBindTexture(GL_TEXTURE_2D, transientTexture.texture->get());

				renderInstances(defaultShader, texturedQuad, std::span{&transientTexture.instance, 1});
			}
		}

		// Render quads.
		if (!renderPass.quadsSortedByShaderAndTexture.empty()) {
			const RenderPass::TexturedQuadInstancesFromQuad& firstQuads = renderPass.quadsSortedByShaderAndTexture.front();

			Shader2D* shader = firstQuads.shader.get();
			Shader2D* actualShader = (shader) ? shader : &defaultShader;
			useShader(*actualShader, projectionViewMatrix);

			const Texture* texture = firstQuads.texture.get();
			const Texture* actualTexture = (texture) ? texture : &whiteTexture;
			glBindTexture(GL_TEXTURE_2D, actualTexture->get());

			renderInstances(*actualShader, texturedQuad, firstQuads.instances);

			for (const RenderPass::TexturedQuadInstancesFromQuad& quads : std::span{renderPass.quadsSortedByShaderAndTexture}.subspan(1)) {
				if (shader != quads.shader.get()) {
					shader = quads.shader.get();
					actualShader = (shader) ? shader : &defaultShader;
					useShader(*actualShader, projectionViewMatrix);
				}

				if (texture != quads.texture.get()) {
					texture = quads.texture.get();
					actualTexture = (texture) ? texture : &whiteTexture;
					glBindTexture(GL_TEXTURE_2D, actualTexture->get());
				}

				renderInstances(*actualShader, texturedQuad, quads.instances);
			}
		}

		// Render sprites.
		if (!renderPass.spritesSortedByAtlas.empty()) {
			const RenderPass::TexturedQuadInstancesFromSprite& firstSprites = renderPass.spritesSortedByAtlas.front();

			useShader(defaultShader, projectionViewMatrix);

			const SpriteAtlas* atlas = firstSprites.atlas.get();
			assert(atlas);
			glBindTexture(GL_TEXTURE_2D, atlas->getAtlasTexture().get());

			renderInstances(defaultShader, texturedQuad, firstSprites.instances);

			for (const RenderPass::TexturedQuadInstancesFromSprite& sprites : std::span{renderPass.spritesSortedByAtlas}.subspan(1)) {
				if (atlas != sprites.atlas.get()) {
					atlas = sprites.atlas.get();
					assert(atlas);
					glBindTexture(GL_TEXTURE_2D, atlas->getAtlasTexture().get());
				}

				renderInstances(defaultShader, texturedQuad, sprites.instances);
			}
		}

		// Render glyphs.
		if (!renderPass.glyphsSortedByFont.empty()) {
			const RenderPass::TexturedQuadInstancesFromText& firstGlyphs = renderPass.glyphsSortedByFont.front();

			useShader(glyphShader, projectionViewMatrix);

			const Font* font = firstGlyphs.font.get();
			assert(font);
			glBindTexture(GL_TEXTURE_2D, font->getAtlasTexture().get());

			renderInstances(defaultShader, texturedQuad, firstGlyphs.instances);

			for (const RenderPass::TexturedQuadInstancesFromText& glyphs : std::span{renderPass.glyphsSortedByFont}.subspan(1)) {
				if (font != glyphs.font.get()) {
					font = glyphs.font.get();
					assert(font);
					glBindTexture(GL_TEXTURE_2D, font->getAtlasTexture().get());
				}

				renderInstances(defaultShader, texturedQuad, glyphs.instances);
			}
		}
	}
}

} // namespace graphics
} // namespace donut
