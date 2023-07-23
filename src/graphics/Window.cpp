#include <donut/donut.hpp>
#include <donut/graphics/Error.hpp>
#include <donut/graphics/Window.hpp>
#include <donut/graphics/opengl.hpp>

//
#include <SDL.h> // SDL..., Uint32
#ifdef __EMSCRIPTEN__
#include <SDL_opengles.h> // SDL_GL_...
#else
#include <SDL_opengl.h> // SDL_GL_...
#endif
//

#include <cstdint> // std::uint32_t
#include <format>  // std::format

namespace donut::graphics {

Window::Window(const WindowOptions& options) {
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
		throw Error{std::format("Failed to initialize SDL video subsystem:\n{}", SDL_GetError())};
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#ifdef __EMSCRIPTEN__
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, options.msaaLevel > 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, std::max(0, options.msaaLevel));

	Uint32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
	if (options.resizable) {
		windowFlags |= SDL_WINDOW_RESIZABLE;
	}
	if (options.fullscreen) {
		windowFlags |= SDL_WINDOW_FULLSCREEN;
	}

	window.reset(SDL_CreateWindow(options.title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, options.size.x, options.size.y, windowFlags));
	if (!window) {
		throw Error{std::format("Failed to create window: {}", SDL_GetError())};
	}

	glContext.reset(SDL_GL_CreateContext(static_cast<SDL_Window*>(window.get())));
	if (!glContext) {
		throw Error{std::format("Failed to create OpenGL context: {}", SDL_GetError())};
	}

#ifndef __EMSCRIPTEN__
	if (gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress) == 0) { // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
		throw Error{"Failed to load OpenGL!"};
	}
#endif

	setVSync(options.vSync);
}

Window::~Window() {
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void Window::present() {
	SDL_GL_SwapWindow(static_cast<SDL_Window*>(window.get()));
}

void Window::setTitle(const char* title) {
	SDL_SetWindowTitle(static_cast<SDL_Window*>(window.get()), title);
}

void Window::setSize(ivec2 size) {
	SDL_SetWindowSize(static_cast<SDL_Window*>(window.get()), size.x, size.y);
}

void Window::setResizable(bool resizable) {
	SDL_SetWindowResizable(static_cast<SDL_Window*>(window.get()), (resizable) ? SDL_TRUE : SDL_FALSE);
}

void Window::setFullscreen(bool fullscreen) {
	if (fullscreen && SDL_GetWindowFlags(static_cast<SDL_Window*>(window.get())) & SDL_WINDOW_RESIZABLE) {
		const int displayIndex = SDL_GetWindowDisplayIndex(static_cast<SDL_Window*>(window.get()));
		if (displayIndex < 0) {
			throw Error{std::format("Failed to get window display index: {}", SDL_GetError())};
		}
		SDL_DisplayMode desktopDisplayMode{};
		if (SDL_GetDesktopDisplayMode(displayIndex, &desktopDisplayMode) != 0) {
			throw Error{std::format("Failed to get desktop display mode: {}", SDL_GetError())};
		}
		if (SDL_SetWindowDisplayMode(static_cast<SDL_Window*>(window.get()), &desktopDisplayMode) != 0) {
			throw Error{std::format("Failed to set window display mode: {}", SDL_GetError())};
		}
	}
	if (SDL_SetWindowFullscreen(static_cast<SDL_Window*>(window.get()), (fullscreen) ? SDL_WINDOW_FULLSCREEN : 0) != 0) {
		throw Error{std::format("Failed to set fullscreen: {}", SDL_GetError())};
	}
}

void Window::setVSync(bool vSync) {
	if (vSync) {
		if (SDL_GL_SetSwapInterval(-1) != 0 && SDL_GL_SetSwapInterval(1) != 0) {
			throw Error{std::format("Failed to enable VSync: {}", SDL_GetError())};
		}
	} else {
		if (SDL_GL_SetSwapInterval(0) != 0) {
			throw Error{std::format("Failed to disable VSync: {}", SDL_GetError())};
		}
	}
}

[[nodiscard]] bool Window::isScreenKeyboardShown() const noexcept {
	return SDL_IsScreenKeyboardShown(static_cast<SDL_Window*>(window.get()));
}

[[nodiscard]] bool Window::isFullscreen() const noexcept {
	const Uint32 flags = SDL_GetWindowFlags(static_cast<SDL_Window*>(window.get()));
	return (flags & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) != 0;
}

[[nodiscard]] ivec2 Window::getSize() const noexcept {
	int width = 0;
	int height = 0;
	SDL_GetWindowSize(static_cast<SDL_Window*>(window.get()), &width, &height);
	return {width, height};
}

[[nodiscard]] ivec2 Window::getDrawableSize() const noexcept {
	int width = 0;
	int height = 0;
	SDL_GL_GetDrawableSize(static_cast<SDL_Window*>(window.get()), &width, &height);
	return {width, height};
}

[[nodiscard]] std::uint32_t Window::getId() const {
	const Uint32 id = SDL_GetWindowID(static_cast<SDL_Window*>(window.get()));
	if (id == 0) {
		throw Error{std::format("Failed to get window ID: {}", SDL_GetError())};
	}
	return id;
}

[[nodiscard]] Framebuffer& Window::getFramebuffer() {
	if (SDL_GL_MakeCurrent(static_cast<SDL_Window*>(window.get()), static_cast<SDL_GLContext>(glContext.get())) != 0) {
		throw Error{std::format("Failed to make OpenGL context current: {}", SDL_GetError())};
	}
	return framebuffer;
}

void Window::WindowDeleter::operator()(void* handle) const noexcept {
	SDL_DestroyWindow(static_cast<SDL_Window*>(handle));
}

void Window::GLContextDeleter::operator()(void* handle) const noexcept {
	SDL_GL_DeleteContext(static_cast<SDL_GLContext>(handle));
}

} // namespace donut::graphics
