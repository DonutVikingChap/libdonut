#ifndef DONUT_GRAPHICS_CAMERA_HPP
#define DONUT_GRAPHICS_CAMERA_HPP

#include <glm/ext/matrix_clip_space.hpp> // glm::ortho, glm::perspective
#include <glm/ext/matrix_transform.hpp>  // glm::identity, glm::lookAt
#include <glm/glm.hpp>                   // glm::...

namespace donut {
namespace graphics {

/**
 * Configuration options for a Camera with an orthographic projection.
 */
struct CameraOrthographicOptions {
	glm::vec2 offset{0.0f, 0.0f}; ///< Bottom left corner of the orthographic projection, in framebuffer coordinates.
	glm::vec2 size{1.0f, 1.0f};   ///< Size of the orthographic projection, in framebuffer coordinates.
};

/**
 * Configuration options for a Camera with a perspective projection.
 */
struct CameraPerspectiveOptions {
	float verticalFieldOfView = 1.57079633f; ///< Vertical field of view of the projection, in radians.
	float aspectRatio = 1.0f;                ///< Aspect ratio of the projection, X/Y.
	float nearZ = 0.01f;                     ///< Distance to the near plane of the projection, in view coordinates.
	float farZ = 1000.0f;                    ///< Distance to the far plane of the projection, in view coordinates.
};

/**
 * Combined view-projection matrix, defining the perspective for a Renderer to
 * render from.
 */
class Camera {
public:
	/**
	 * Create a camera with an orthographic projection.
	 *
	 * \param options projection options, see CameraOrthographicOptions.
	 * \param viewMatrix view matrix of the camera.
	 *
	 * \return an orthographic camera with the specified configuration.
	 */
	[[nodiscard]] static Camera createOrthographic(const CameraOrthographicOptions& options, const glm::mat4& viewMatrix) noexcept {
		return Camera{glm::ortho(options.offset.x, options.offset.x + options.size.x, options.offset.y, options.offset.y + options.size.y), viewMatrix};
	}

	/**
	 * Create a camera with an orthographic projection and an identity view
	 * matrix at the default position.
	 *
	 * \param options projection options, see CameraOrthographicOptions.
	 *
	 * \return an orthographic camera with the specified configuration.
	 */
	[[nodiscard]] static Camera createOrthographic(const CameraOrthographicOptions& options) noexcept {
		return createOrthographic(options, glm::identity<glm::mat4>());
	}

	/**
	 * Create a camera with an orthographic projection.
	 *
	 * \param options projection options, see CameraOrthographicOptions.
	 * \param position position of the camera, in world coordinates.
	 * \param target target position to aim the camera at, in world coordinates.
	 * \param up up direction of the camera in the world.
	 *
	 * \return an orthographic camera with the specified configuration.
	 */
	[[nodiscard]] static Camera createOrthographic(const CameraOrthographicOptions& options, glm::vec3 position, glm::vec3 target, glm::vec3 up) noexcept {
		return createOrthographic(options, glm::lookAt(position, target, up));
	}
	/**
	 * Create a camera with a perspective projection.
	 *
	 * \param options projection options, see CameraPerspectiveOptions.
	 * \param viewMatrix view matrix of the camera.
	 *
	 * \return an orthographic camera with the specified configuration.
	 */
	[[nodiscard]] static Camera createPerspective(const CameraPerspectiveOptions& options, const glm::mat4& viewMatrix) noexcept {
		return Camera{glm::perspective(options.verticalFieldOfView, options.aspectRatio, options.nearZ, options.farZ), viewMatrix};
	}

	/**
	 * Create a camera with a perspective projection and an identity view
	 * matrix at the default position.
	 *
	 * \param options projection options, see CameraPerspectiveOptions.
	 *
	 * \return an orthographic camera with the specified configuration.
	 */
	[[nodiscard]] static Camera createPerspective(const CameraPerspectiveOptions& options) noexcept {
		return createPerspective(options, glm::identity<glm::mat4>());
	}

	/**
	 * Create a camera with a perspective projection.
	 *
	 * \param options projection options, see CameraPerspectiveOptions.
	 * \param position position of the camera, in world coordinates.
	 * \param target target position to aim the camera at, in world coordinates.
	 * \param up up direction of the camera in the world.
	 *
	 * \return an orthographic camera with the specified configuration.
	 */
	[[nodiscard]] static Camera createPerspective(const CameraPerspectiveOptions& options, glm::vec3 position, glm::vec3 target, glm::vec3 up) noexcept {
		return createPerspective(options, glm::lookAt(position, target, up));
	}

	/**
	 * Construct a camera with an identity projection matrix and view matrix.
	 */
	constexpr Camera() noexcept
		: projectionMatrix(glm::identity<glm::mat4>())
		, viewMatrix(glm::identity<glm::mat4>()) {}

	/**
	 * Construct a camera with a specific projection matrix and view matrix.
	 *
	 * \param projectionMatrix projection matrix of the camera.
	 * \param viewMatrix view matrix of the camera.
	 */
	constexpr Camera(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix) noexcept
		: projectionMatrix(projectionMatrix)
		, viewMatrix(viewMatrix) {}

	/**
	 * Set the projection of the camera to an orthographic projection.
	 *
	 * \param options projection options, see CameraOrthographicOptions.
	 */
	void setProjectionOrthographic(const CameraOrthographicOptions& options) noexcept {
		setProjectionMatrix(glm::ortho(options.offset.x, options.offset.x + options.size.x, options.offset.y, options.offset.y + options.size.y));
	}

	/**
	 * Set the projection of the camera to a perspective projection.
	 *
	 * \param options projection options, see CameraPerspectiveOptions.
	 */
	void setProjectionPerspective(const CameraPerspectiveOptions& options) noexcept {
		setProjectionMatrix(glm::perspective(options.verticalFieldOfView, options.aspectRatio, options.nearZ, options.farZ));
	}

	/**
	 * Set the view of the camera.
	 *
	 * \param newPosition new position of the camera, in world coordinates.
	 * \param newTarget new target position to aim the camera at, in world
	 *        coordinates.
	 * \param newUp new up direction of the camera in the world.
	 */
	void setView(glm::vec3 newPosition, glm::vec3 newTarget, glm::vec3 newUp) noexcept {
		setViewMatrix(glm::lookAt(newPosition, newTarget, newUp));
	}

	/**
	 * Set the projection matrix of the camera.
	 *
	 * \param newProjectionMatrix new projection matrix to set.
	 */
	void setProjectionMatrix(const glm::mat4& newProjectionMatrix) noexcept {
		projectionMatrix = newProjectionMatrix;
	}

	/**
	 * Set the view matrix of the camera.
	 *
	 * \param newViewMatrix new view matrix to set.
	 */
	void setViewMatrix(const glm::mat4& newViewMatrix) noexcept {
		viewMatrix = newViewMatrix;
	}

	/**
	 * Get the projection matrix of the camera.
	 *
	 * \return the camera's current projection matrix.
	 */
	[[nodiscard]] const glm::mat4& getProjectionMatrix() const noexcept {
		return projectionMatrix;
	}

	/**
	 * Get the view matrix of the camera.
	 *
	 * \return the camera's current view matrix.
	 */
	[[nodiscard]] const glm::mat4& getViewMatrix() const noexcept {
		return viewMatrix;
	}

private:
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
};

} // namespace graphics
} // namespace donut

#endif
