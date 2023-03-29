#include <donut/application/Application.hpp>
#include <donut/application/Error.hpp>
#include <donut/graphics/opengl.hpp>

//
#include <SDL.h> // SDL...
#include <SDL_messagebox.h>
#ifdef __EMSCRIPTEN__
#include <SDL_opengles.h> // SDL_GL_...
#else
#include <SDL_opengl.h> // SDL_GL_...
#endif
//
#ifdef __EMSCRIPTEN__
#include <emscripten.h> // emscripten_...
#endif
//

#include <algorithm>    // std::min, std::max
#include <cmath>        // std::ceil
#include <cstdint>      // std::uint64_t, std::uint32_t
#include <cstdio>       // stderr
#include <exception>    // std::exception
#include <fmt/format.h> // fmt::format, fmt::print
#include <glm/glm.hpp>  // glm::...
#include <physfs.h>     // PHYSFS_...
#include <string>       // std::string
#include <string_view>  // std::string_view

namespace donut {
namespace application {

void Application::showSimpleMessageBox(MessageBoxType type, const char* title, const char* message) {
	std::uint32_t flags = 0;
	switch (type) {
		case MessageBoxType::ERROR_MESSAGE: flags = SDL_MESSAGEBOX_ERROR; break;
		case MessageBoxType::WARNING_MESSAGE: flags = SDL_MESSAGEBOX_WARNING; break;
		case MessageBoxType::INFO_MESSAGE: flags = SDL_MESSAGEBOX_INFORMATION; break;
	}
	if (SDL_ShowSimpleMessageBox(flags, title, message, nullptr) != 0) {
		throw std::runtime_error{fmt::format("Failed to show simple message box: {}", SDL_GetError())};
	}
}

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

	std::uint32_t windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
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
	setFrameRateParameters(options.tickRate, options.minFps, options.maxFps);
}

void Application::run() {
	startClockTime = SDL_GetPerformanceCounter();
	latestFrameClockTime = startClockTime;
	latestFpsMeasurementClockTime = startClockTime;
	processedTickClockTime = startClockTime;
	latestMeasuredFps = 0u;
	fpsCounter = 0u;
	frameInfo.tickInfo.processedTickCount = 0u;
	frameInfo.tickInfo.processedTickTime = 0.0f;
	frameInfo.tickInterpolationAlpha = 0.0f;
	frameInfo.elapsedTime = 0.0f;
	frameInfo.deltaTime = 0.0f;

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
		try {
			runFrame();
		} catch (...) {
			quit();
			throw;
		}
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
	const std::uint32_t flags = SDL_GetWindowFlags(static_cast<SDL_Window*>(window.get()));
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
	if (fullscreen && SDL_GetWindowFlags(static_cast<SDL_Window*>(window.get())) & SDL_WINDOW_RESIZABLE) {
		const int displayIndex = SDL_GetWindowDisplayIndex(static_cast<SDL_Window*>(window.get()));
		if (displayIndex < 0) {
			throw Error{fmt::format("Failed to get window display index: {}", SDL_GetError())};
		}
		SDL_DisplayMode desktopDisplayMode{};
		if (SDL_GetDesktopDisplayMode(displayIndex, &desktopDisplayMode) != 0) {
			throw Error{fmt::format("Failed to get desktop display mode: {}", SDL_GetError())};
		}
		if (SDL_SetWindowDisplayMode(static_cast<SDL_Window*>(window.get()), &desktopDisplayMode) != 0) {
			throw Error{fmt::format("Failed to set window display mode: {}", SDL_GetError())};
		}
	}
	if (SDL_SetWindowFullscreen(static_cast<SDL_Window*>(window.get()), (fullscreen) ? SDL_WINDOW_FULLSCREEN : 0) != 0) {
		throw Error{fmt::format("Failed to set fullscreen: {}", SDL_GetError())};
	}
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

void Application::setFrameRateParameters(float tickRate, float minFps, float maxFps) {
	tickClockInterval = static_cast<std::uint64_t>(std::ceil(static_cast<float>(clockFrequency) / tickRate));
	frameInfo.tickInfo.tickInterval = static_cast<float>(tickClockInterval) * clockInterval;
	minFrameClockInterval = (maxFps == 0.0f) ? 0 : static_cast<std::uint64_t>(std::ceil(static_cast<float>(clockFrequency) / maxFps));
	maxTicksPerFrame = (minFps <= 0.0f || tickRate <= minFps) ? 1 : static_cast<std::uint64_t>(tickRate / minFps);
}

void Application::runFrame() {
	const std::uint64_t currentClockTime = SDL_GetPerformanceCounter();
	const std::uint64_t clockDeltaTime = currentClockTime - latestFrameClockTime;
	if (currentClockTime > latestFrameClockTime && clockDeltaTime >= minFrameClockInterval) {
		latestFrameClockTime = currentClockTime;
		++fpsCounter;
		if (currentClockTime - latestFpsMeasurementClockTime >= clockFrequency) {
			latestFpsMeasurementClockTime = currentClockTime;
			latestMeasuredFps = fpsCounter;
			fpsCounter = 0;
		}

		frameInfo.elapsedTime = static_cast<float>(currentClockTime - startClockTime) * clockInterval;
		frameInfo.deltaTime = static_cast<float>(clockDeltaTime) * clockInterval;

		prepareForEvents(frameInfo);
		for (SDL_Event event{}; SDL_PollEvent(&event) != 0;) {
			switch (event.type) {
				case SDL_QUIT: quit(); return;
				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
						resize({event.window.data1, event.window.data2});
					}
					break;
				default: break;
			}
			handleEvent(frameInfo, event);
		}

		update(frameInfo);
		const std::uint64_t clockTimeSinceLatestTick = currentClockTime - processedTickClockTime;
		for (std::uint64_t ticksToProcess = std::min(clockTimeSinceLatestTick / tickClockInterval, maxTicksPerFrame); ticksToProcess > 0; --ticksToProcess) {
			tick(frameInfo.tickInfo);
			++frameInfo.tickInfo.processedTickCount;
			frameInfo.tickInfo.processedTickTime += frameInfo.tickInfo.tickInterval;
			processedTickClockTime += tickClockInterval;
		}

		frameInfo.tickInterpolationAlpha = std::min(1.0f, (static_cast<float>(currentClockTime - processedTickClockTime) * clockInterval) / frameInfo.tickInfo.tickInterval);

		prepareForDisplay(frameInfo);
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
			if (organizationName && applicationName) {
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
			}
#endif

			if (dataDirectoryFilepath) {
				if (PHYSFS_mount(dataDirectoryFilepath, nullptr, 1) == 0) {
					throw Error{fmt::format("Failed to set application data directory: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
				}
			}

			if (archiveFilenameExtension) {
				PHYSFS_enumerate(
					"/",
					[](void* data, const char*, const char* fname) -> PHYSFS_EnumerateCallbackResult {
						const std::string_view archiveFilenameExtension{static_cast<const char*>(data)};
						const std::string_view filename{fname};
						if (!archiveFilenameExtension.empty() && filename.size() > archiveFilenameExtension.size() &&
							(archiveFilenameExtension.front() == '.' || filename[filename.size() - archiveFilenameExtension.size() - 1] == '.') &&
							filename.ends_with(archiveFilenameExtension)) {
							if (const char* directory = PHYSFS_getRealDir(fname)) {
								const std::string archiveFilepath = fmt::format("{}{}{}", directory, PHYSFS_getDirSeparator(), fname);
								if (PHYSFS_mount(archiveFilepath.c_str(), nullptr, 0) == 0) {
									fmt::print(stderr, "Failed to mount archive \"{}\": {}\n", archiveFilepath, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
								}
							} else {
								fmt::print(stderr, "Failed to get the real directory of archive \"{}\": {}\n", fname, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
							}
						}
						return PHYSFS_ENUM_OK;
					},
					const_cast<char*>(archiveFilenameExtension));
			}
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
