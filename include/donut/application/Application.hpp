#ifndef DONUT_APPLICATION_APPLICATION_HPP
#define DONUT_APPLICATION_APPLICATION_HPP

#include <donut/Resource.hpp>
#include <donut/application/Event.hpp>

#include <cstdint>
#include <glm/glm.hpp>

namespace donut {
namespace application {

struct FrameInfo {
	unsigned tickCount;
	float latestTickTime;
	float elapsedTime;
	float deltaTime;
};

struct TickInfo {
	unsigned tickCount;
	float latestTickTime;
	float tickInterval;
};

struct ApplicationOptions {
	const char* organizationName = "Unknown";
	const char* applicationName = "Application";
	const char* dataDirectoryFilepath = ".";
	const char* archiveFilenameExtension = "pak";
	const char* windowTitle = "Application";
	int windowWidth = 800;
	int windowHeight = 600;
	bool windowResizable = true;
	bool windowFullscreen = false;
	bool windowVSync = false;
	float tickRate = 60.0f;
	float minFps = 10.0f;
	float maxFps = 60.0f;
	int msaaLevel = 0;
};

class Application {
public:
	explicit Application(const char* programFilepath, const ApplicationOptions& options);
	virtual ~Application() = default;

	void run();
	void quit() noexcept;

	[[nodiscard]] bool isRunning() const noexcept;
	[[nodiscard]] bool isWindowFullscreen() const noexcept;
	[[nodiscard]] unsigned getLatestMeasuredFps() const noexcept;
	[[nodiscard]] glm::ivec2 getWindowSize() const noexcept;

	void setWindowTitle(const char* title);
	void setWindowSize(glm::ivec2 size);
	void setWindowResizable(bool resizable);
	void setWindowFullscreen(bool fullscreen);
	void setWindowVSync(bool vSync);
	void setWindowFramerate(float tickRate, float minFps, float maxFps);

private:
	virtual void resize(glm::ivec2 newWindowSize) = 0;
	virtual void beginFrame(const FrameInfo& frameInfo) = 0;
	virtual void handleEvent(const FrameInfo& frameInfo, const Event& event) = 0;
	virtual void update(const FrameInfo& frameInfo) = 0;
	virtual void tick(const TickInfo& tickInfo) = 0;
	virtual void endFrame(const FrameInfo& frameInfo) = 0;
	virtual void display(const FrameInfo& frameInfo) = 0;

	void runFrame();

	struct PhysFSManager {
		PhysFSManager(
			const char* programFilepath, const char* organizationName, const char* applicationName, const char* dataDirectoryFilepath, const char* archiveFilenameExtension);
	};

	struct SDLManager {
		SDLManager();
	};

	struct WindowDeleter {
		void operator()(void* handle) const noexcept;
	};

	using Window = Resource<void*, WindowDeleter, nullptr>;

	struct GLContextDeleter {
		void operator()(void* handle) const noexcept;
	};

	using GLContext = Resource<void*, GLContextDeleter, nullptr>;

	[[no_unique_address]] PhysFSManager physFSManager;
	[[no_unique_address]] SDLManager sdlManager{};
	Window window{};
	GLContext glContext{};
	std::uint64_t clockFrequency = 0;
	std::uint64_t tickInterval = 0;
	std::uint64_t minFrameInterval = 0;
	std::uint64_t maxTicksPerFrame = 0;
	std::uint64_t startTime = 0;
	std::uint64_t latestTickTime = 0;
	std::uint64_t latestFrameTime = 0;
	std::uint64_t latestFpsCountTime = 0;
	float clockInterval = 0.0f;
	float tickDeltaTime = 0.0f;
	unsigned latestMeasuredFps = 0u;
	unsigned tickCount = 0u;
	unsigned fpsCount = 0u;
	bool running = false;
};

} // namespace application
} // namespace donut

#endif
