#include <donut/Variant.hpp>
#include <donut/graphics/Framebuffer.hpp>
#include <donut/graphics/Model.hpp>
#include <donut/graphics/Renderer.hpp>
#include <donut/graphics/Shader.hpp>
#include <donut/graphics/Shader2D.hpp>
#include <donut/graphics/Shader3D.hpp>
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

void applyShaderOptions(const auto& options) {
	if (options.overwriteDepthBuffer) {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);
	} else if (options.useDepthTest) {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
	} else {
		glDisable(GL_DEPTH_TEST);
	}

	glDisable(GL_STENCIL_TEST);

	if (options.useBackfaceCulling) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
	} else {
		glDisable(GL_CULL_FACE);
	}

	if (options.useAlphaBlending) {
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	} else {
		glDisable(GL_BLEND);
	}
}

void uploadShaderUniforms(ShaderProgram& program) {
	for (const auto& [location, value] : program.getUniformUploadQueue()) {
		const GLint loc = location;
		match(value)( //
			[loc](float v) -> void { glUniform1f(loc, v); },
			[loc](glm::vec2 v) -> void { glUniform2f(loc, v.x, v.y); },
			[loc](glm::vec3 v) -> void { glUniform3f(loc, v.x, v.y, v.z); },
			[loc](glm::vec4 v) -> void { glUniform4f(loc, v.x, v.y, v.z, v.w); },
			[loc](glm::i32 v) -> void { glUniform1i(loc, v); },
			[loc](glm::i32vec2 v) -> void { glUniform2i(loc, v.x, v.y); },
			[loc](glm::i32vec3 v) -> void { glUniform3i(loc, v.x, v.y, v.z); },
			[loc](glm::i32vec4 v) -> void { glUniform4i(loc, v.x, v.y, v.z, v.w); },
			[loc](glm::u32 v) -> void { glUniform1ui(loc, v); },
			[loc](glm::u32vec2 v) -> void { glUniform2ui(loc, v.x, v.y); },
			[loc](glm::u32vec3 v) -> void { glUniform3ui(loc, v.x, v.y, v.z); },
			[loc](glm::u32vec4 v) -> void { glUniform4ui(loc, v.x, v.y, v.z, v.w); },
			[loc](const glm::mat2& v) -> void { glUniformMatrix2fv(loc, 1, GL_FALSE, glm::value_ptr(v)); },
			[loc](const glm::mat3& v) -> void { glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(v)); },
			[loc](const glm::mat4& v) -> void { glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(v)); });
	}
	program.clearUniformUploadQueue();
}

void useShader(Shader3D& shader, const glm::mat4& projectionViewMatrix) {
	glUseProgram(shader.program.get());
	applyShaderOptions(shader.options);
	uploadShaderUniforms(shader.program);
	glUniformMatrix4fv(shader.projectionViewMatrix.getLocation(), 1, GL_FALSE, glm::value_ptr(projectionViewMatrix));
	glUniform1i(shader.diffuseMap.getLocation(), Model::Object::TEXTURE_UNIT_DIFFUSE);
	glUniform1i(shader.specularMap.getLocation(), Model::Object::TEXTURE_UNIT_SPECULAR);
	glUniform1i(shader.normalMap.getLocation(), Model::Object::TEXTURE_UNIT_NORMAL);
}

void useShader(Shader2D& shader, const glm::mat4& projectionViewMatrix) {
	glUseProgram(shader.program.get());
	applyShaderOptions(shader.options);
	uploadShaderUniforms(shader.program);
	glUniformMatrix4fv(shader.projectionViewMatrix.getLocation(), 1, GL_FALSE, glm::value_ptr(projectionViewMatrix));
	glUniform1i(shader.textureUnit.getLocation(), TexturedQuad::TEXTURE_UNIT);
}

void renderModelInstances(Shader3D& shader, std::span<const Model::Object> objects, std::span<const Model::Object::Instance> instances) {
	for (const Model::Object& object : objects) {
		glBindVertexArray(object.mesh.get());
		glBindBuffer(GL_ARRAY_BUFFER, object.mesh.getInstanceBuffer());

		glActiveTexture(GL_TEXTURE0 + Model::Object::TEXTURE_UNIT_DIFFUSE);
		glBindTexture(GL_TEXTURE_2D, (object.material.diffuseMap) ? object.material.diffuseMap.get() : Texture::whiteR8G8B8A8Srgb1x1->get());

		glActiveTexture(GL_TEXTURE0 + Model::Object::TEXTURE_UNIT_SPECULAR);
		glBindTexture(GL_TEXTURE_2D, (object.material.specularMap) ? object.material.specularMap.get() : Texture::grayR8G8B8A8Unorm1x1->get());

		glActiveTexture(GL_TEXTURE0 + Model::Object::TEXTURE_UNIT_NORMAL);
		glBindTexture(GL_TEXTURE_2D, (object.material.normalMap) ? object.material.normalMap.get() : Texture::normalR8G8B8Unorm1x1->get());

		glUniform1f(shader.specularExponent.getLocation(), object.material.specularExponent);

		glBufferData(
			GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(instances.size() * sizeof(Model::Object::Instance)), instances.data(), static_cast<GLenum>(Model::Object::INSTANCES_USAGE));
		glDrawElementsInstanced(static_cast<GLenum>(Model::Object::PRIMITIVE_TYPE),
			static_cast<GLsizei>(object.indexCount),
			static_cast<GLenum>(Model::Object::INDEX_TYPE),
			nullptr,
			static_cast<GLsizei>(instances.size()));
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
	Framebuffer& framebuffer, const RenderPass& renderPass, const Viewport& viewport, const glm::mat4& projectionViewMatrix) {
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
				useShader(*shader, projectionViewMatrix);
			}

			renderModelInstances(*shader, key.model->objects, model.instances);
		}
	}

	// Render 2D objects.
	{
		glBindVertexArray(texturedQuad.mesh.get());
		glBindBuffer(GL_ARRAY_BUFFER, texturedQuad.mesh.getInstanceBuffer());

		glActiveTexture(GL_TEXTURE0 + TexturedQuad::TEXTURE_UNIT);

		// Render quads.
		Shader2D* shader = nullptr;
		const Texture* texture = nullptr;
		for (const RenderPass::TexturedQuadInstances& quad : renderPass.quads) {
			if (shader != quad.shader) {
				shader = quad.shader;
				useShader(*shader, projectionViewMatrix);
			}

			if (texture != quad.texture) {
				texture = quad.texture;
				glBindTexture(GL_TEXTURE_2D, texture->get());
			}

			renderTexturedQuadInstances(quad.instances);
		}
	}
}

} // namespace graphics
} // namespace donut
