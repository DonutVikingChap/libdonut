#include <donut/application/Application.hpp>
#include <donut/application/Error.hpp>
#include <donut/graphics/opengl.hpp>

//
#include <SDL.h>
#ifdef __EMSCRIPTEN__
#include <SDL_opengles.h>
#else
#include <SDL_opengl.h>
#endif
//
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
//

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <fmt/format.h>
#include <glm/glm.hpp>
#include <physfs.h>

namespace donut {
namespace application {

Application::Application(const char* programFilepath, const ApplicationOptions& options)
	: physFSManager(programFilepath, options.organizationName, options.applicationName, options.dataDirectoryFilepath, options.archiveFilenameExtension)
	, clockFrequency(SDL_GetPerformanceFrequency())
	, clockInterval(1.0f / static_cast<float>(clockFrequency)) {
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
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
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, options.msaaLevel);

	Uint32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
	if (options.windowResizable) {
		windowFlags |= SDL_WINDOW_RESIZABLE;
	}
	if (options.windowFullscreen) {
		windowFlags |= SDL_WINDOW_FULLSCREEN;
	}

	window = Window{SDL_CreateWindow(options.windowTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, options.windowWidth, options.windowHeight, windowFlags)};
	if (!window) {
		throw Error{fmt::format("Failed to create window: {}", SDL_GetError())};
	}

	glContext = GLContext{SDL_GL_CreateContext(static_cast<SDL_Window*>(window.get()))};
	if (!glContext) {
		throw Error{fmt::format("Failed to create OpenGL context: {}", SDL_GetError())};
	}

#ifndef __EMSCRIPTEN__
	if (gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress) == 0) { // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
		throw Error{"Failed to load OpenGL!"};
	}
#endif

	setWindowVSync(options.windowVSync);
	setWindowFramerate(options.tickRate, options.minFps, options.maxFps);
}

void Application::run() {
	startTime = SDL_GetPerformanceCounter();
	latestTickTime = startTime;
	latestFrameTime = startTime;
	latestFpsCountTime = startTime;

	resize(getWindowSize());

	running = true;

#ifdef __EMSCRIPTEN__
	constexpr auto run_emscripten_frame = [](void* arg) -> void {
		Application* const application = static_cast<Application*>(arg);
		try {
			application->runFrame();
		} catch (const std::exception& e) {
			fmt::print(stderr, "Fatal error: {}\n", e.what());
			application->quit();
		} catch (...) {
			fmt::print(stderr, "Fatal error!\n");
			application->quit();
		}
		if (!application->isRunning()) {
			application->~Application();
			emscripten_cancel_main_loop();
		}
	};
	emscripten_set_main_loop_arg(run_emscripten_frame, this, 0, 1);
#else
	while (isRunning()) {
		runFrame();
	}
#endif
}

void Application::quit() noexcept {
	running = false;
}

bool Application::isRunning() const noexcept {
	return running;
}

bool Application::isWindowFullscreen() const noexcept {
	const Uint32 flags = SDL_GetWindowFlags(static_cast<SDL_Window*>(window.get()));
	return (flags & SDL_WINDOW_FULLSCREEN) != 0 || (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
}

unsigned Application::getLatestMeasuredFps() const noexcept {
	return latestMeasuredFps;
}

glm::ivec2 Application::getWindowSize() const noexcept {
	int width = 0;
	int height = 0;
	SDL_GetWindowSize(static_cast<SDL_Window*>(window.get()), &width, &height);
	return {width, height};
}

void Application::setWindowTitle(const char* title) {
	SDL_SetWindowTitle(static_cast<SDL_Window*>(window.get()), title);
}

void Application::setWindowSize(glm::ivec2 size) {
	SDL_SetWindowSize(static_cast<SDL_Window*>(window.get()), size.x, size.y);
}

void Application::setWindowResizable(bool resizable) {
	SDL_SetWindowResizable(static_cast<SDL_Window*>(window.get()), (resizable) ? SDL_TRUE : SDL_FALSE);
}

void Application::setWindowFullscreen(bool fullscreen) {
	if (SDL_SetWindowFullscreen(static_cast<SDL_Window*>(window.get()), (fullscreen) ? SDL_WINDOW_FULLSCREEN : 0) != 0) {
		throw Error{fmt::format("Failed to set fullscreen: {}", SDL_GetError())};
	}
	resize(getWindowSize());
}

void Application::setWindowVSync(bool vSync) {
	if (vSync) {
		if (SDL_GL_SetSwapInterval(-1) != 0 && SDL_GL_SetSwapInterval(1) != 0) {
			throw Error{fmt::format("Failed to enable VSync: {}", SDL_GetError())};
		}
	} else {
		if (SDL_GL_SetSwapInterval(0) != 0) {
			throw Error{fmt::format("Failed to disable VSync: {}", SDL_GetError())};
		}
	}
}

void Application::setWindowFramerate(float tickRate, float minFps, float maxFps) {
	tickInterval = static_cast<std::uint64_t>(std::ceil(static_cast<float>(clockFrequency) / tickRate));
	tickDeltaTime = static_cast<float>(tickInterval) * clockInterval;
	minFrameInterval = (maxFps == 0.0f) ? 0 : static_cast<std::uint64_t>(std::ceil(static_cast<float>(clockFrequency) / maxFps));
	maxTicksPerFrame = (tickRate <= minFps) ? 1 : static_cast<std::uint64_t>(tickRate / minFps);
}

void Application::runFrame() {
	const std::uint64_t currentTime = SDL_GetPerformanceCounter();
	const std::uint64_t timeSinceLatestFrame = currentTime - latestFrameTime;
	if (currentTime > latestFrameTime && timeSinceLatestFrame >= minFrameInterval) {
		latestFrameTime = currentTime;
		++fpsCount;
		if (currentTime - latestFpsCountTime >= clockFrequency) {
			latestFpsCountTime = currentTime;
			latestMeasuredFps = fpsCount;
			fpsCount = 0;
		}

		FrameInfo frameInfo{
			.tickCount = tickCount,
			.latestTickTime = static_cast<float>(latestTickTime) * clockInterval,
			.elapsedTime = static_cast<float>(currentTime - startTime) * clockInterval,
			.deltaTime = static_cast<float>(timeSinceLatestFrame) * clockInterval,
		};

		beginFrame(frameInfo);

		for (SDL_Event event{}; SDL_PollEvent(&event) != 0;) {
			switch (event.type) {
				case SDL_QUIT: quit(); return;
				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
						resize({event.window.data1, event.window.data2});
					}
					break;
				default: break;
			}
			handleEvent(frameInfo, event);
		}

		update(frameInfo);

		const std::uint64_t timeSinceLatestTick = currentTime - latestTickTime;
		for (std::uint64_t ticks = std::min(timeSinceLatestTick / tickInterval, maxTicksPerFrame); ticks > 0; --ticks) {
			tick({
				.tickCount = tickCount,
				.latestTickTime = static_cast<float>(latestTickTime) * clockInterval,
				.tickInterval = tickDeltaTime,
			});
			++tickCount;
			latestTickTime += tickInterval;
		}
		frameInfo.tickCount = tickCount;
		frameInfo.latestTickTime = static_cast<float>(latestTickTime) * clockInterval;

		endFrame(frameInfo);

		display(frameInfo);
		SDL_GL_SwapWindow(static_cast<SDL_Window*>(window.get()));
	}
}

Application::PhysFSManager::PhysFSManager(
	const char* programFilepath, const char* organizationName, const char* applicationName, const char* dataDirectoryFilepath, const char* archiveFilenameExtension) {
	struct Context {
		[[nodiscard]] Context(
			const char* programFilepath, const char* organizationName, const char* applicationName, const char* dataDirectoryFilepath, const char* archiveFilenameExtension) {
			if (PHYSFS_init(programFilepath) == 0) {
				throw Error{fmt::format("Failed to initialize PhysicsFS: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
			}

#ifndef __EMSCRIPTEN__
			const char* const prefDir = PHYSFS_getPrefDir(organizationName, applicationName);
			if (!prefDir) {
				throw Error{fmt::format("Failed to get application preferences directory: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
			}

			if (PHYSFS_setWriteDir(prefDir) == 0) {
				throw Error{fmt::format("Failed to set application write directory: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
			}

			if (PHYSFS_mount(prefDir, nullptr, 0) == 0) {
				throw Error{fmt::format("Failed to mount application write directory: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
			}
#endif

			if (PHYSFS_mount(dataDirectoryFilepath, nullptr, 1) == 0) {
				throw Error{fmt::format("Failed to set application data directory: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
			}

			PHYSFS_enumerate(
				"/",
				[](void* data, const char*, const char* fname) -> PHYSFS_EnumerateCallbackResult {
					const std::string_view archiveFilenameExtension{static_cast<const char*>(data)};
					const std::string_view filename{fname};
					if (filename.size() > archiveFilenameExtension.size() && filename[filename.size() - archiveFilenameExtension.size() - 1] == '.' &&
						filename.ends_with(archiveFilenameExtension)) {
						if (const char* directory = PHYSFS_getRealDir(fname)) {
							PHYSFS_mount(fmt::format("{}{}{}", directory, PHYSFS_getDirSeparator(), fname).c_str(), nullptr, 0);
						}
					}
					return PHYSFS_ENUM_OK;
				},
				const_cast<char*>(archiveFilenameExtension));
		}

		~Context() {
			PHYSFS_deinit();
		}

		Context(const Context&) = delete;
		Context(Context&&) = delete;
		Context& operator=(const Context&) = delete;
		Context& operator=(Context&&) = delete;
	};
	static const Context context{programFilepath, organizationName, applicationName, dataDirectoryFilepath, archiveFilenameExtension};
}

Application::SDLManager::SDLManager() {
	struct Context {
		[[nodiscard]] Context() {
			if (SDL_Init(SDL_INIT_VIDEO) != 0) {
				throw Error{fmt::format("Failed to initialize SDL: {}", SDL_GetError())};
			}
		}

		~Context() {
			SDL_Quit();
		}

		Context(const Context&) = delete;
		Context(Context&&) = delete;
		Context& operator=(const Context&) = delete;
		Context& operator=(Context&&) = delete;
	};
	static const Context context{};
}

void Application::WindowDeleter::operator()(void* handle) const noexcept {
	SDL_DestroyWindow(static_cast<SDL_Window*>(handle));
}

void Application::GLContextDeleter::operator()(void* handle) const noexcept {
	SDL_GL_DeleteContext(static_cast<SDL_GLContext>(handle));
}

} // namespace application
} // namespace donut
