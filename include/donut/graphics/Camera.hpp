#ifndef DONUT_GRAPHICS_CAMERA_HPP
#define DONUT_GRAPHICS_CAMERA_HPP

#include <donut/math.hpp>

namespace donut::graphics {

/**
 * Configuration options for a Camera with an orthographic projection.
 */
struct CameraOrthographicOptions {
	/**
	 * Bottom left corner of the orthographic projection, in framebuffer
	 * coordinates.
	 */
	vec2 offset{0.0f, 0.0f};

	/**
	 * Size of the orthographic projection, in framebuffer coordinates.
	 */
	vec2 size{1.0f, 1.0f};
};

/**
 * Configuration options for a Camera with a perspective projection.
 */
struct CameraPerspectiveOptions {
	/**
	 * Vertical field of view of the projection, in radians.
	 */
	float verticalFieldOfView = 1.28700221758656877361f;

	/**
	 * Aspect ratio of the projection, X/Y.
	 */
	float aspectRatio = 1.0f;

	/**
	 * Distance to the near plane of the projection, in view coordinates.
	 */
	float nearZ = 0.01f;

	/**
	 * Distance to the far plane of the projection, in view coordinates.
	 */
	float farZ = 1000.0f;
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
	[[nodiscard]] static Camera createOrthographic(const CameraOrthographicOptions& options, const mat4& viewMatrix) noexcept {
		return Camera{ortho(options.offset.x, options.offset.x + options.size.x, options.offset.y, options.offset.y + options.size.y), viewMatrix};
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
		return createOrthographic(options, identity<mat4>());
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
	[[nodiscard]] static Camera createOrthographic(const CameraOrthographicOptions& options, vec3 position, vec3 target, vec3 up) noexcept {
		return createOrthographic(options, lookAt(position, target, up));
	}
	/**
	 * Create a camera with a perspective projection.
	 *
	 * \param options projection options, see CameraPerspectiveOptions.
	 * \param viewMatrix view matrix of the camera.
	 *
	 * \return an orthographic camera with the specified configuration.
	 */
	[[nodiscard]] static Camera createPerspective(const CameraPerspectiveOptions& options, const mat4& viewMatrix) noexcept {
		return Camera{perspective(options.verticalFieldOfView, options.aspectRatio, options.nearZ, options.farZ), viewMatrix};
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
		return createPerspective(options, identity<mat4>());
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
	[[nodiscard]] static Camera createPerspective(const CameraPerspectiveOptions& options, vec3 position, vec3 target, vec3 up) noexcept {
		return createPerspective(options, lookAt(position, target, up));
	}

	/**
	 * Construct a camera with an identity projection matrix and view matrix.
	 */
	constexpr Camera() noexcept
		: projectionMatrix(identity<mat4>())
		, viewMatrix(identity<mat4>()) {}

	/**
	 * Construct a camera with a specific projection matrix and view matrix.
	 *
	 * \param projectionMatrix projection matrix of the camera.
	 * \param viewMatrix view matrix of the camera.
	 */
	constexpr Camera(const mat4& projectionMatrix, const mat4& viewMatrix) noexcept
		: projectionMatrix(projectionMatrix)
		, viewMatrix(viewMatrix) {}

	/**
	 * Set the projection of the camera to an orthographic projection.
	 *
	 * \param options projection options, see CameraOrthographicOptions.
	 */
	void setProjectionOrthographic(const CameraOrthographicOptions& options) noexcept {
		setProjectionMatrix(ortho(options.offset.x, options.offset.x + options.size.x, options.offset.y, options.offset.y + options.size.y));
	}

	/**
	 * Set the projection of the camera to a perspective projection.
	 *
	 * \param options projection options, see CameraPerspectiveOptions.
	 */
	void setProjectionPerspective(const CameraPerspectiveOptions& options) noexcept {
		setProjectionMatrix(perspective(options.verticalFieldOfView, options.aspectRatio, options.nearZ, options.farZ));
	}

	/**
	 * Set the view of the camera.
	 *
	 * \param newPosition new position of the camera, in world coordinates.
	 * \param newTarget new target position to aim the camera at, in world
	 *        coordinates.
	 * \param newUp new up direction of the camera in the world.
	 */
	void setView(vec3 newPosition, vec3 newTarget, vec3 newUp) noexcept {
		setViewMatrix(lookAt(newPosition, newTarget, newUp));
	}

	/**
	 * Set the projection matrix of the camera.
	 *
	 * \param newProjectionMatrix new projection matrix to set.
	 */
	void setProjectionMatrix(const mat4& newProjectionMatrix) noexcept {
		projectionMatrix = newProjectionMatrix;
	}

	/**
	 * Set the view matrix of the camera.
	 *
	 * \param newViewMatrix new view matrix to set.
	 */
	void setViewMatrix(const mat4& newViewMatrix) noexcept {
		viewMatrix = newViewMatrix;
	}

	/**
	 * Get the projection matrix of the camera.
	 *
	 * \return the camera's current projection matrix.
	 */
	[[nodiscard]] const mat4& getProjectionMatrix() const noexcept {
		return projectionMatrix;
	}

	/**
	 * Get the view matrix of the camera.
	 *
	 * \return the camera's current view matrix.
	 */
	[[nodiscard]] const mat4& getViewMatrix() const noexcept {
		return viewMatrix;
	}

private:
	mat4 projectionMatrix;
	mat4 viewMatrix;
};

} // namespace donut::graphics

#endif
