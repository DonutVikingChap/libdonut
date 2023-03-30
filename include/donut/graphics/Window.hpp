#ifndef DONUT_GRAPHICS_WINDOW_HPP
#define DONUT_GRAPHICS_WINDOW_HPP

#include <donut/Resource.hpp>
#include <donut/graphics/Framebuffer.hpp>
#include <donut/graphics/Handle.hpp>

#include <cstdint>     // std::uint32_t
#include <glm/glm.hpp> // glm::...

namespace donut {
namespace graphics {

/**
 * Unique identifier corresponding to a particular window.
 */
using WindowId = std::uint32_t;

/**
 * Configuration options for a Window.
 */
struct WindowOptions {
	/**
	 * Non-owning pointer to a null-terminated UTF-8 string of the displayed
	 * title of the window.
	 *
	 * \warning Must not be set to nullptr.
	 */
	const char* title = "Application";

	/**
	 * The desired size of the window, in screen coordinates (typically pixels).
	 *
	 * The x component represents the width, and the y component represents the
	 * height.
	 *
	 * \warning Both the width and height must be positive.
	 */
	glm::ivec2 size{800, 600};

	/**
	 * Whether the user should be allowed to resize the window or not.
	 */
	bool resizable = true;

	/**
	 * Whether the window should start in fullscreen mode or not.
	 */
	bool fullscreen = false;

	/**
	 * Whether the window should use vertical synchronization or not.
	 *
	 * VSync introduces a pause each time an application frame is finished
	 * rendering that causes the application to wait for the previously
	 * displayed frame to finish being drawn to the screen. This eliminates any
	 * tearing artifacts that may otherwise occur due to swapping the frame
	 * buffers in the middle of a screen refresh, at the cost of effectively
	 * limiting the application's frame rate to the screen's refresh rate.
	 *
	 * \note Enabling VSync is not recommended for applications which are
	 *       sensitive to input delay, such as games, since it can significantly
	 *       increase the time before a rendered frame gets displayed to the
	 *       user compared to a regular frame rate limiter.
	 */
	bool vSync = false;

	/**
	 * Number of samples used for multisample anti-aliasing (MSAA) when
	 * rendering a pixel to the window via its Framebuffer.
	 *
	 * This can be used to mitigate aliasing artifacts on the edges of 3D
	 * objects, at the cost of some performance.
	 *
	 * If set to 0 or lower, MSAA will not be used.
	 *
	 * \remark Typical values are 0, 2 and 4.
	 *
	 * \note Current GPUs (as of 2023) rarely support values greater than 8.
	 */
	int msaaLevel = 0;
};

/**
 * Graphical window that can be rendered to.
 */
class Window {
public:
	/**
	 * Create a new window.
	 *
	 * \param options initial configuration of the window, see WindowOptions.
	 *
	 * \throws graphics::Error if context or window setup failed.
	 * \throws std::bad_alloc on allocation failure.
	 */
	explicit Window(const WindowOptions& options);

	/**
	 * Swap the window's front and back buffers, showing what has been rendered
	 * to the framebuffer since the last presentation.
	 */
	void present();

	/**
	 * Set the displayed title of the window.
	 *
	 * \param title non-owning pointer to a null-terminated UTF-8 string
	 *        containing the title. Must not be nullptr.
	 *
	 * \sa WindowOptions::title
	 */
	void setTitle(const char* title);

	/**
	 * Set the size of the window.
	 *
	 * \param size a 2D vector representing the desired size of the window, in
	 *        screen coordinates (typically pixels), where:
	 *        - the x component represents the width, and
	 *        - the y component represents the height.
	 *        Both the width and height must be positive.
	 *
	 * \sa WindowOptions::size
	 */
	void setSize(glm::ivec2 size);

	/**
	 * Set whether to allow the window to be resized by the user or not.
	 *
	 * \param resizable true to allow resizing, false to disallow.
	 *
	 * \sa WindowOptions::resizable
	 */
	void setResizable(bool resizable);

	/**
	 * Set the fullscreen state of the window.
	 *
	 * \param fullscreen true for fullscreen mode, false for windowed mode.
	 *
	 * \sa WindowOptions::fullscreen
	 */
	void setFullscreen(bool fullscreen);

	/**
	 * Enable or disable vertical synchronization for the window.
	 *
	 * \param vSync true to enable VSync, false to disable.
	 *
	 * \sa WindowOptions::vSync
	 */
	void setVSync(bool vSync);

	/**
	 * Check if the screen keyboard is currently open.
	 *
	 * \return true if the screen keyboard is open, false otherwise.
	 *
	 * \sa application::Application::hasScreenKeyboardSupport()
	 */
	[[nodiscard]] bool isScreenKeyboardShown() const noexcept;

	/**
	 * Check if the window is currently in fullscreen mode.
	 *
	 * \return true if the window is in fullscreen mode, false otherwise.
	 */
	[[nodiscard]] bool isFullscreen() const noexcept;

	/**
	 * Get the size of the window.
	 *
	 * \return a 2D vector representing the current size of the window, in
	 *         screen coordinates (typically pixels), where:
	 *         - the x component represents the width, and
	 *         - the y component represents the height.
	 */
	[[nodiscard]] glm::ivec2 getSize() const noexcept;

	/**
	 * Get the drawable size of the window.
	 *
	 * \return a 2D vector representing the current drawable size of the window,
	 *         in pixels, where:
	 *         - the x component represents the width, and
	 *         - the y component represents the height.
	 */
	[[nodiscard]] glm::ivec2 getDrawableSize() const noexcept;

	/**
	 * Get a unique identifier for this window.
	 *
	 * \return the identifier corresponding to this window.
	 */
	[[nodiscard]] WindowId getId() const;

	/**
	 * Get the Framebuffer for rendering to this window.
	 *
	 * \return a reference to the default framebuffer, with the GL context
	 *         associated with this window made current.
	 */
	[[nodiscard]] Framebuffer& getFramebuffer();

private:
	struct WindowDeleter {
		void operator()(void* handle) const noexcept;
	};

	using WindowObject = Resource<void*, WindowDeleter, nullptr>;

	struct GLContextDeleter {
		void operator()(void* handle) const noexcept;
	};

	using GLContext = Resource<void*, GLContextDeleter, nullptr>;

	WindowObject window{};
	GLContext glContext{};
	Framebuffer framebuffer{Handle{}};
};

} // namespace graphics
} // namespace donut

#endif
