#include <donut/Variant.hpp>
#include <donut/graphics/Framebuffer.hpp>
#include <donut/graphics/Model.hpp>
#include <donut/graphics/Renderer.hpp>
#include <donut/graphics/Shader2D.hpp>
#include <donut/graphics/Shader3D.hpp>
#include <donut/graphics/ShaderConfiguration.hpp>
#include <donut/graphics/ShaderProgram.hpp>
#include <donut/graphics/SpriteAtlas.hpp>
#include <donut/graphics/Texture.hpp>
#include <donut/graphics/TexturedQuad.hpp>
#include <donut/graphics/Viewport.hpp>
#include <donut/graphics/opengl.hpp>
#include <donut/math.hpp>

#include <cassert> // assert
#include <cstddef> // std::size_t
#include <span>    // std::span
#include <vector>  // std::vector

namespace donut::graphics {

namespace {

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

void renderModelInstances(Shader3D& shader, std::span<const Model::Object> objects, std::span<const Model::Object::Instance> instances) {
	for (const Model::Object& object : objects) {
		glBindVertexArray(object.mesh.get());
		glBindBuffer(GL_ARRAY_BUFFER, object.mesh.getInstanceBuffer());

		glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + Model::Object::TEXTURE_UNIT_DIFFUSE));
		glBindTexture(GL_TEXTURE_2D, (object.material.diffuseMap) ? object.material.diffuseMap.get() : Texture::defaultWhite->get());

		glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + Model::Object::TEXTURE_UNIT_SPECULAR));
		glBindTexture(GL_TEXTURE_2D, (object.material.specularMap) ? object.material.specularMap.get() : Texture::defaultGray->get());

		glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + Model::Object::TEXTURE_UNIT_NORMAL));
		glBindTexture(GL_TEXTURE_2D, (object.material.normalMap) ? object.material.normalMap.get() : Texture::defaultNormal->get());

		glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + Model::Object::TEXTURE_UNIT_EMISSIVE));
		glBindTexture(GL_TEXTURE_2D, (object.material.emissiveMap) ? object.material.emissiveMap.get() : Texture::defaultTransparent->get());

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
	} catch (...) {
		Shader2D::destroySharedShaders();
		throw;
	}
	try {
		Texture::createSharedTextures();
	} catch (...) {
		Shader3D::destroySharedShaders();
		Shader2D::destroySharedShaders();
		throw;
	}
}

Renderer::~Renderer() {
	Texture::destroySharedTextures();
	Shader3D::destroySharedShaders();
	Shader2D::destroySharedShaders();
}

void Renderer::clearFramebufferDepth(Framebuffer& framebuffer) {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.get());
	glClear(GL_DEPTH_BUFFER_BIT);
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

void Renderer::render( // NOLINT(readability-make-member-function-const)
	Framebuffer& framebuffer, const RenderPass& renderPass, const Viewport& viewport, const Camera& camera) {
	// Bind framebuffer.
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.get());

	// Setup viewport.
	glViewport(viewport.position.x, viewport.position.y, viewport.size.x, viewport.size.y);

	// Render 3D objects.
	{
		// Render models.
		Shader3D* shader = nullptr;
		for (const auto& [key, model] : renderPass.models) {
			if (shader != key.shader) {
				shader = key.shader;
				useShader(*shader);
				uploadCameraToShader(*shader, camera);
			}

			renderModelInstances(*shader, key.model->objects, model.instances);
		}
	}

	// Render 2D objects.
	{
		glBindVertexArray(texturedQuad.mesh.get());
		glBindBuffer(GL_ARRAY_BUFFER, texturedQuad.mesh.getInstanceBuffer());

		glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + TexturedQuad::TEXTURE_UNIT));

		// Render quads.
		Shader2D* shader = nullptr;
		const Texture* texture = nullptr;
		for (const RenderPass::TexturedQuadInstances& quad : renderPass.quads) {
			if (shader != quad.shader) {
				shader = quad.shader;
				useShader(*shader);
				uploadCameraToShader(*shader, camera);
			}

			if (texture != quad.texture) {
				texture = quad.texture;
				glBindTexture(GL_TEXTURE_2D, texture->get());
			}

			renderTexturedQuadInstances(quad.instances);
		}
	}
}

} // namespace donut::graphics
