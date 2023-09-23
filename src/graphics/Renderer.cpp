#include <donut/Overloaded.hpp>
#include <donut/Variant.hpp>
#include <donut/graphics/Font.hpp>
#include <donut/graphics/Framebuffer.hpp>
#include <donut/graphics/Handle.hpp>
#include <donut/graphics/Model.hpp>
#include <donut/graphics/RenderPass.hpp>
#include <donut/graphics/Renderer.hpp>
#include <donut/graphics/Shader2D.hpp>
#include <donut/graphics/Shader3D.hpp>
#include <donut/graphics/ShaderConfiguration.hpp>
#include <donut/graphics/ShaderProgram.hpp>
#include <donut/graphics/SpriteAtlas.hpp>
#include <donut/graphics/Text.hpp>
#include <donut/graphics/Texture.hpp>
#include <donut/graphics/TexturedQuad.hpp>
#include <donut/graphics/Viewport.hpp>
#include <donut/graphics/opengl.hpp>
#include <donut/math.hpp>

#include <cassert>     // assert
#include <cstddef>     // std::size_t
#include <span>        // std::span
#include <string_view> // std::string_view
#include <utility>     // std::move

namespace donut::graphics {

namespace {

void useFramebuffer(Framebuffer& framebuffer) {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.get());
}

void useViewport(const Viewport& viewport) {
	glViewport(viewport.position.x, viewport.position.y, viewport.size.x, viewport.size.y);
}

void useScissor(std::optional<Rectangle<int>> scissor) {
	if (scissor) {
		glEnable(GL_SCISSOR_TEST);
		glScissor(scissor->position.x, scissor->position.y, scissor->size.x, scissor->size.y);
	} else {
		glDisable(GL_SCISSOR_TEST);
	}
}

void useTexturedQuad(const TexturedQuad& texturedQuad) {
	glBindVertexArray(texturedQuad.mesh.get());
	glBindBuffer(GL_ARRAY_BUFFER, texturedQuad.mesh.getInstanceBuffer());

	glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + TexturedQuad::TEXTURE_UNIT));
}

void applyShaderConfiguration(const ShaderConfiguration& configuration) {
	switch (configuration.depthBufferMode) {
		case DepthBufferMode::IGNORE: glDisable(GL_DEPTH_TEST); break;
		case DepthBufferMode::USE_DEPTH_TEST:
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(static_cast<GLenum>(configuration.depthTestPredicate));
			break;
	}

	switch (configuration.stencilBufferMode) {
		case StencilBufferMode::IGNORE: glDisable(GL_STENCIL_TEST); break;
		case StencilBufferMode::USE_STENCIL_TEST:
			glEnable(GL_STENCIL_TEST);
			glStencilFunc(static_cast<GLenum>(configuration.stencilTestPredicate), static_cast<GLint>(configuration.stencilTestReferenceValue),
				static_cast<GLuint>(configuration.stencilTestMask));
			glStencilOp(static_cast<GLenum>(configuration.stencilBufferOperationOnStencilTestFail), static_cast<GLenum>(configuration.stencilBufferOperationOnDepthTestFail),
				static_cast<GLenum>(configuration.stencilBufferOperationOnPass));
			break;
	}

	switch (configuration.faceCullingMode) {
		case FaceCullingMode::IGNORE: glDisable(GL_CULL_FACE); break;
		case FaceCullingMode::CULL_BACK_FACES:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(static_cast<GLenum>(configuration.frontFace));
			break;
		case FaceCullingMode::CULL_FRONT_FACES:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			glFrontFace(static_cast<GLenum>(configuration.frontFace));
			break;
		case FaceCullingMode::CULL_FRONT_AND_BACK_FACES:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT_AND_BACK);
			glFrontFace(static_cast<GLenum>(configuration.frontFace));
			break;
	}

	switch (configuration.alphaMode) {
		case AlphaMode::IGNORE: glDisable(GL_BLEND); break;
		case AlphaMode::USE_ALPHA_BLENDING:
			glEnable(GL_BLEND);
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			break;
	}
}

void uploadEnqueuedShaderUniformValues(ShaderProgram& program) {
	for (const auto& [location, value] : program.getUniformUploadQueue()) {
		const GLint loc = location;
		match(value)(                                                                             //
			[loc](float v) -> void { glUniform1f(loc, v); },                                      //
			[loc](vec2 v) -> void { glUniform2f(loc, v.x, v.y); },                                //
			[loc](vec3 v) -> void { glUniform3f(loc, v.x, v.y, v.z); },                           //
			[loc](vec4 v) -> void { glUniform4f(loc, v.x, v.y, v.z, v.w); },                      //
			[loc](i32 v) -> void { glUniform1i(loc, v); },                                        //
			[loc](i32vec2 v) -> void { glUniform2i(loc, v.x, v.y); },                             //
			[loc](i32vec3 v) -> void { glUniform3i(loc, v.x, v.y, v.z); },                        //
			[loc](i32vec4 v) -> void { glUniform4i(loc, v.x, v.y, v.z, v.w); },                   //
			[loc](u32 v) -> void { glUniform1ui(loc, v); },                                       //
			[loc](u32vec2 v) -> void { glUniform2ui(loc, v.x, v.y); },                            //
			[loc](u32vec3 v) -> void { glUniform3ui(loc, v.x, v.y, v.z); },                       //
			[loc](u32vec4 v) -> void { glUniform4ui(loc, v.x, v.y, v.z, v.w); },                  //
			[loc](const mat2& v) -> void { glUniformMatrix2fv(loc, 1, GL_FALSE, value_ptr(v)); }, //
			[loc](const mat3& v) -> void { glUniformMatrix3fv(loc, 1, GL_FALSE, value_ptr(v)); }, //
			[loc](const mat4& v) -> void { glUniformMatrix4fv(loc, 1, GL_FALSE, value_ptr(v)); });
	}
	program.clearUniformUploadQueue();
}

void bindTextures(ShaderProgram& program, std::int32_t textureUnitOffset) {
	for (const auto& [location, texture] : program.getTextureBindings()) {
		assert(texture);
		glUniform1i(location, textureUnitOffset);
		glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + textureUnitOffset));
		glBindTexture(GL_TEXTURE_2D, texture->get());
		++textureUnitOffset;
	}
}

void useShader(Shader3D& shader) {
	glUseProgram(shader.program.get());
	applyShaderConfiguration(shader.options.configuration);
	uploadEnqueuedShaderUniformValues(shader.program);
	glUniform1i(shader.diffuseMap.getLocation(), Model::Object::TEXTURE_UNIT_DIFFUSE);
	glUniform1i(shader.specularMap.getLocation(), Model::Object::TEXTURE_UNIT_SPECULAR);
	glUniform1i(shader.normalMap.getLocation(), Model::Object::TEXTURE_UNIT_NORMAL);
	glUniform1i(shader.emissiveMap.getLocation(), Model::Object::TEXTURE_UNIT_EMISSIVE);
	bindTextures(shader.program, Model::Object::TEXTURE_UNIT_COUNT);
}

void useShader(Shader2D& shader) {
	glUseProgram(shader.program.get());
	applyShaderConfiguration(shader.options.configuration);
	uploadEnqueuedShaderUniformValues(shader.program);
	glUniform1i(shader.textureUnit.getLocation(), TexturedQuad::TEXTURE_UNIT);
	bindTextures(shader.program, TexturedQuad::TEXTURE_UNIT_COUNT);
}

void uploadCameraToShader(auto& shader, const Camera& camera) {
	glUniformMatrix4fv(shader.projectionMatrix.getLocation(), 1, GL_FALSE, value_ptr(camera.getProjectionMatrix()));
	glUniformMatrix4fv(shader.viewMatrix.getLocation(), 1, GL_FALSE, value_ptr(camera.getViewMatrix()));
	glUniformMatrix4fv(shader.viewProjectionMatrix.getLocation(), 1, GL_FALSE, value_ptr(camera.getProjectionMatrix() * camera.getViewMatrix()));
}

void useTexture(const Texture& texture) {
	glBindTexture(GL_TEXTURE_2D, texture.get());
}

void renderModelInstances(Shader3D& shader, const Texture* diffuseMapOverride, const Texture* specularMapOverride, const Texture* normalMapOverride,
	const Texture* emissiveMapOverride, std::span<const Model::Object> objects, std::span<const Model::Object::Instance> instances) {
	for (const Model::Object& object : objects) {
		const Handle diffuseMapTextureHandle =
			(diffuseMapOverride)           ? diffuseMapOverride->get()
			: (object.material.diffuseMap) ? object.material.diffuseMap.get()
										   : Texture::WHITE->get();

		const Handle specularMapTextureHandle =
			(specularMapOverride)           ? specularMapOverride->get()
			: (object.material.specularMap) ? object.material.specularMap.get()
											: Texture::DEFAULT_SPECULAR->get();

		const Handle normalMapTextureHandle =
			(normalMapOverride)           ? normalMapOverride->get()
			: (object.material.normalMap) ? object.material.normalMap.get()
										  : Texture::DEFAULT_NORMAL->get();

		const Handle emissiveMapTextureHandle =
			(emissiveMapOverride)           ? emissiveMapOverride->get()
			: (object.material.emissiveMap) ? object.material.emissiveMap.get()
											: Texture::WHITE->get();

		glBindVertexArray(object.mesh.get());
		glBindBuffer(GL_ARRAY_BUFFER, object.mesh.getInstanceBuffer());

		glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + Model::Object::TEXTURE_UNIT_DIFFUSE));
		glBindTexture(GL_TEXTURE_2D, diffuseMapTextureHandle);

		glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + Model::Object::TEXTURE_UNIT_SPECULAR));
		glBindTexture(GL_TEXTURE_2D, specularMapTextureHandle);

		glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + Model::Object::TEXTURE_UNIT_NORMAL));
		glBindTexture(GL_TEXTURE_2D, normalMapTextureHandle);

		glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + Model::Object::TEXTURE_UNIT_EMISSIVE));
		glBindTexture(GL_TEXTURE_2D, emissiveMapTextureHandle);

		glUniform3fv(shader.diffuseColor.getLocation(), 1, value_ptr(object.material.diffuseColor));
		glUniform3fv(shader.specularColor.getLocation(), 1, value_ptr(object.material.specularColor));
		glUniform3fv(shader.normalScale.getLocation(), 1, value_ptr(object.material.normalScale));
		glUniform3fv(shader.emissiveColor.getLocation(), 1, value_ptr(object.material.emissiveColor));
		glUniform1f(shader.specularExponent.getLocation(), object.material.specularExponent);
		glUniform1f(shader.dissolveFactor.getLocation(), object.material.dissolveFactor);
		glUniform1f(shader.occlusionFactor.getLocation(), object.material.occlusionFactor);

		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(instances.size() * sizeof(Model::Object::Instance)), instances.data(),
			static_cast<GLenum>(Model::Object::INSTANCES_USAGE));
		glDrawElementsInstanced(static_cast<GLenum>(Model::Object::PRIMITIVE_TYPE), static_cast<GLsizei>(object.indexCount), static_cast<GLenum>(Model::Object::INDEX_TYPE),
			nullptr, static_cast<GLsizei>(instances.size()));
	}
}

void renderTexturedQuadInstances(std::span<const TexturedQuad::Instance> instances) {
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(instances.size() * sizeof(TexturedQuad::Instance)), instances.data(), static_cast<GLenum>(TexturedQuad::INSTANCES_USAGE));
	glDrawArraysInstanced(static_cast<GLenum>(TexturedQuad::PRIMITIVE_TYPE), 0, static_cast<GLsizei>(TexturedQuad::VERTICES.size()), static_cast<GLsizei>(instances.size()));
}

} // namespace

Renderer::Renderer(const RendererOptions& /*options*/) {
	Shader2D::createSharedShaders();
	try {
		Shader3D::createSharedShaders();
		try {
			Texture::createSharedTextures();
			try {
				Model::createSharedModels();
			} catch (...) {
				Texture::destroySharedTextures();
				throw;
			}
		} catch (...) {
			Shader3D::destroySharedShaders();
			throw;
		}
	} catch (...) {
		Shader2D::destroySharedShaders();
		throw;
	}
}

Renderer::~Renderer() {
	Model::destroySharedModels();
	Texture::destroySharedTextures();
	Shader3D::destroySharedShaders();
	Shader2D::destroySharedShaders();
}

void Renderer::clearFramebufferDepth(Framebuffer& framebuffer) {
	useFramebuffer(framebuffer);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void Renderer::clearFramebufferColor(Framebuffer& framebuffer, Color color) {
	useFramebuffer(framebuffer);
	glClearColor(color.getRedComponent(), color.getGreenComponent(), color.getBlueComponent(), color.getAlphaComponent());
	glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::clearFramebufferColorAndDepth(Framebuffer& framebuffer, Color color) {
	useFramebuffer(framebuffer);
	glClearColor(color.getRedComponent(), color.getGreenComponent(), color.getBlueComponent(), color.getAlphaComponent());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::render(Framebuffer& framebuffer, const RenderPass& renderPass, const Viewport& viewport, const Camera& camera, std::optional<Rectangle<int>> scissor) {
	for (Font* const font : renderPass.fonts) {
		font->renderMarkedGlyphs(*this);
	}

	useFramebuffer(framebuffer);
	useViewport(viewport);
	useScissor(scissor);

	{
		Shader3D* boundShader3D = nullptr;
		Shader2D* boundShader2D = nullptr;
		const Model* boundModel = nullptr;
		const Texture* boundDiffuseMapOverride = nullptr;
		const Texture* boundSpecularMapOverride = nullptr;
		const Texture* boundNormalMapOverride = nullptr;
		const Texture* boundEmissiveMapOverride = nullptr;
		const Texture* boundTexture = nullptr;
		const SpriteAtlas* boundSpriteAtlas = nullptr;
		Font* boundFont = nullptr;

		const auto pushModelInstance = [&](const mat4& transformation, vec2 textureOffset, vec2 textureScale, Color tintColor, vec3 specularFactor, vec3 emissiveFactor) -> void {
			modelInstances.push_back(Model::Object::Instance{
				.transformation = transformation,
				.normalMatrix = inverseTranspose(mat3{transformation}),
				.textureOffsetAndScale{textureOffset.x, textureOffset.y, textureScale.x, textureScale.y},
				.tintColor = tintColor,
				.specularFactor = specularFactor,
				.emissiveFactor = emissiveFactor,
			});
		};

		const auto pushTexturedQuadInstance = [&](const mat3& transformation, vec2 textureOffset, vec2 textureScale, Color tintColor) -> void {
			assert(boundShader2D);
			assert(boundTexture);
			texturedQuadInstances.push_back(TexturedQuad::Instance{
				.transformation = transformation,
				.textureOffsetAndScale{textureOffset.x, textureOffset.y, textureScale.x, textureScale.y},
				.tintColor = tintColor,
			});
		};

		const auto pushRectangleInstance = [&](vec2 position, float angle, vec2 size, vec2 origin, vec2 textureOffset, vec2 textureScale, Color tintColor) -> void {
			assert(boundShader2D);
			assert(boundTexture);
			const mat3 translation{vec3{1.0f, 0.0f, 0.0f}, vec3{0.0f, 1.0f, 0.0f}, vec3{position, 1.0f}};
			const mat3 rotation = orientate2(angle);
			const mat3 scale{vec3{size.x, 0.0f, 0.0f}, vec3{0.0f, size.y, 0.0f}, vec3{0.0f, 0.0f, 1.0f}};
			const mat3 originTranslation{vec3{1.0f, 0.0f, 0.0f}, vec3{0.0f, 1.0f, 0.0f}, vec3{-origin, 1.0f}};
			const mat3 transformation = translation * rotation * scale * originTranslation;
			pushTexturedQuadInstance(transformation, textureOffset, textureScale, tintColor);
		};

		const auto pushGlyphInstance = [&](vec2 position, const Text::ShapedGlyph& shapedGlyph, vec2 textureSize, Color color) -> void {
			assert(boundShader2D);
			assert(boundTexture);
			assert(boundFont);
			const Font::Glyph glyph = boundFont->findGlyph(shapedGlyph.characterSize, shapedGlyph.codePoint);
			assert(glyph.rendered);
			const mat3 translation{vec3{1.0f, 0.0f, 0.0f}, vec3{0.0f, 1.0f, 0.0f}, vec3{position + shapedGlyph.shapedOffset, 1.0f}};
			const mat3 scale{vec3{shapedGlyph.shapedSize.x, 0.0f, 0.0f}, vec3{0.0f, shapedGlyph.shapedSize.y, 0.0f}, vec3{0.0f, 0.0f, 1.0f}};
			const mat3 transformation = translation * scale;
			pushTexturedQuadInstance(transformation, glyph.positionInAtlas / textureSize, glyph.sizeInAtlas / textureSize, color);
		};

		const auto render3DInstances = [&]() -> void {
			if (!modelInstances.empty()) {
				renderModelInstances(*boundShader3D, boundDiffuseMapOverride, boundSpecularMapOverride, boundNormalMapOverride, boundEmissiveMapOverride, boundModel->objects,
					modelInstances);
				modelInstances.clear();
			}
		};

		const auto render2DInstances = [&]() -> void {
			if (!texturedQuadInstances.empty()) {
				renderTexturedQuadInstances(texturedQuadInstances);
				texturedQuadInstances.clear();
			}
		};

		modelInstances.clear();
		texturedQuadInstances.clear();

		renderPass.commandBuffer2D.visit(Overloaded{
			[&](const RenderPass::CommandUseShader3D& command) -> void {
				assert(command.shader);
				render3DInstances();
				render2DInstances();
				boundShader2D = nullptr;
				boundTexture = nullptr;
				boundShader3D = command.shader;
				useShader(*boundShader3D);
				uploadCameraToShader(*boundShader3D, camera);
			},
			[&](const RenderPass::CommandUseShader2D& command) -> void {
				assert(command.shader);
				render3DInstances();
				render2DInstances();
				if (!boundShader2D) {
					useTexturedQuad(texturedQuad);
				}
				boundShader3D = nullptr;
				boundShader2D = command.shader;
				useShader(*boundShader2D);
				uploadCameraToShader(*boundShader2D, camera);
			},
			[&](const RenderPass::CommandUseModel& command) -> void {
				assert(command.model);
				render3DInstances();
				boundModel = command.model;
				boundDiffuseMapOverride = command.diffuseMapOverride;
				boundSpecularMapOverride = command.specularMapOverride;
				boundNormalMapOverride = command.normalMapOverride;
				boundEmissiveMapOverride = command.emissiveMapOverride;
			},
			[&](const RenderPass::CommandUseTexture& command) -> void {
				assert(command.texture);
				render2DInstances();
				boundTexture = command.texture;
				useTexture(*boundTexture);
			},
			[&](const RenderPass::CommandUseSpriteAtlas& command) -> void {
				assert(command.atlas);
				render2DInstances();
				boundSpriteAtlas = command.atlas;
				boundTexture = &boundSpriteAtlas->getAtlasTexture();
				useTexture(*boundTexture);
			},
			[&](const RenderPass::CommandUseFont& command) -> void {
				assert(command.font);
				render2DInstances();
				boundFont = command.font;
				boundTexture = &boundFont->getAtlasTexture();
				useTexture(*boundTexture);
			},
			[&](const RenderPass::CommandDrawModelInstance& command) -> void {
				assert(boundShader3D);
				assert(boundModel);
				pushModelInstance(command.transformation, command.textureOffset, command.textureScale, command.tintColor, command.specularFactor, command.emissiveFactor);
			},
			[&](const RenderPass::CommandDrawQuadInstance& command) -> void {
				assert(boundShader2D);
				assert(boundTexture);
				pushTexturedQuadInstance(command.transformation, command.textureOffset, command.textureScale, command.tintColor);
			},
			[&](const RenderPass::CommandDrawTextureInstance& command) -> void {
				assert(boundShader2D);
				assert(boundTexture);
				pushRectangleInstance(command.position, command.angle, boundTexture->getSize2D() * command.scale, command.origin, command.textureOffset, command.textureScale,
					command.tintColor);
			},
			[&](const RenderPass::CommandDrawRectangleInstance& command) -> void {
				assert(boundShader2D);
				assert(boundTexture);
				pushRectangleInstance(command.position, command.angle, command.size, command.origin, command.textureOffset, command.textureScale, command.tintColor);
			},
			[&](const RenderPass::CommandDrawSpriteInstance& command) -> void {
				assert(boundShader2D);
				assert(boundTexture);
				assert(boundSpriteAtlas);
				const SpriteAtlas::Sprite& sprite = boundSpriteAtlas->getSprite(command.id);
				vec2 positionInAtlas{};
				vec2 sizeInAtlas{};
				if ((sprite.flip & SpriteAtlas::FLIP_HORIZONTALLY) != 0) {
					positionInAtlas.x = sprite.position.x + sprite.size.x;
					sizeInAtlas.x = -sprite.size.x;
				} else {
					positionInAtlas.x = sprite.position.x;
					sizeInAtlas.x = sprite.size.x;
				}
				if ((sprite.flip & SpriteAtlas::FLIP_VERTICALLY) != 0) {
					positionInAtlas.y = sprite.position.y + sprite.size.y;
					sizeInAtlas.y = -sprite.size.y;
				} else {
					positionInAtlas.y = sprite.position.y;
					sizeInAtlas.y = sprite.size.y;
				}
				const vec2 textureSize = boundTexture->getSize2D();
				pushRectangleInstance(command.position, command.angle, sprite.size * command.scale, command.origin, positionInAtlas / textureSize, sizeInAtlas / textureSize,
					command.tintColor);
			},
			[&](const RenderPass::CommandDrawTextInstance& command) -> void {
				assert(boundShader2D);
				assert(command.text);
				for (const Text::ShapedGlyph& shapedGlyph : command.text->getShapedGlyphs()) {
					assert(shapedGlyph.font);
					const Texture* const texture = &shapedGlyph.font->getAtlasTexture();
					if (boundTexture != texture) {
						render2DInstances();
						boundTexture = texture;
						boundFont = shapedGlyph.font;
						useTexture(*boundTexture);
					}
					pushGlyphInstance(command.position, shapedGlyph, texture->getSize2D(), command.color);
				}
			},
			[&](const RenderPass::CommandDrawTextStringInstance& command) -> void {
				assert(boundShader2D);
				assert(boundTexture);
				assert(boundFont);
				text.reshape(*boundFont, command.characterSize, command.string, command.position, command.scale);
				for (const Text::ShapedGlyph& shapedGlyph : text.getShapedGlyphs()) {
					pushGlyphInstance(vec2{0.0f, 0.0f}, shapedGlyph, boundTexture->getSize2D(), command.color);
				}
			},
			[&](std::span<const char>) -> void {},
		});
		render3DInstances();
		render2DInstances();
	}
}

} // namespace donut::graphics
