#include <donut/Time.hpp>
#include <donut/application/Application.hpp>

//
#ifdef __EMSCRIPTEN__
#include <cstdio>       // stderr, std::fprintf
#include <emscripten.h> // emscripten_...
#include <exception>    // std::exception
#endif
//

#include <algorithm> // std::min
#include <chrono>    // std::chrono::...
#include <thread>    // std::this_thread::...

namespace donut::application {

Application::Application(const ApplicationOptions& options) {
	setFrameRateParameters(options.tickRate, options.minFrameRate, options.maxFrameRate);
	setFrameRateLimiterSleepEnabled(options.frameRateLimiterSleepEnabled);
	setFrameRateLimiterSleepBias(options.frameRateLimiterSleepBias);
}

void Application::run() {
	startTime = Clock::now();
	latestFrameTime = startTime;
	latestTickProcessingEndTime = startTime;
	latestFrameCountTime = startTime;
	lastSecondFrameCount = 0u;
	frameCounter = 0u;
	tickInfo.processedTickCount = 0;
	tickInfo.processedTickTime = {};
	frameInfo.tickInterpolationAlpha = 0.0f;
	frameInfo.elapsedTime = {};
	frameInfo.deltaTime = {};
	running = true;

#ifdef __EMSCRIPTEN__
	constexpr auto run_emscripten_frame = [](void* arg) -> void {
		Application* const application = static_cast<Application*>(arg);
		try {
			application->runFrame();
		} catch (const std::exception& e) {
			std::fprintf(stderr, "Fatal error: %s\n", e.what());
			application->quit();
		} catch (...) {
			std::fprintf(stderr, "Fatal error!\n");
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

void Application::quit() {
	running = false;
}

void Application::setFrameRateParameters(float tickRate, float minFrameRate, float maxFrameRate) {
	tickInterval = (tickRate <= 0.0f) ? Clock::duration{} : ceil<Clock::duration>(Time<float>::Duration{1.0f / tickRate});
	tickInfo.tickInterval = duration_cast<decltype(tickInfo.tickInterval)::Duration>(tickInterval);
	minFrameInterval = (maxFrameRate == 0.0f) ? Clock::duration{} : ceil<Clock::duration>(Time<float>::Duration{1.0f / maxFrameRate});
	maxTicksPerFrame =
		(tickRate <= 0.0f)                                   ? Clock::rep{0}
		: (minFrameRate <= 0.0f || tickRate <= minFrameRate) ? Clock::rep{1}
		: (maxFrameRate <= 0.0f || minFrameRate <= maxFrameRate)
			? static_cast<Clock::rep>(tickRate / minFrameRate)
			: static_cast<Clock::rep>(tickRate / maxFrameRate);
	latestTickProcessingEndTime = Clock::now();
}

void Application::setFrameRateLimiterSleepEnabled(bool frameRateLimiterSleepEnabled) {
	this->frameRateLimiterSleepEnabled = frameRateLimiterSleepEnabled;
}

void Application::setFrameRateLimiterSleepBias(std::chrono::steady_clock::duration frameRateLimiterSleepBias) {
	this->frameRateLimiterSleepBias = frameRateLimiterSleepBias;
}

void Application::runFrame() {
	const Clock::time_point currentTime = Clock::now();
	const Clock::duration deltaTime = currentTime - latestFrameTime;
	if (deltaTime < minFrameInterval) {
		if (frameRateLimiterSleepEnabled) {
			std::this_thread::sleep_until(latestFrameTime + minFrameInterval - frameRateLimiterSleepBias);
		}
		return;
	}

	latestFrameTime = currentTime;

	++frameCounter;
	if (currentTime - latestFrameCountTime >= std::chrono::seconds{1}) {
		latestFrameCountTime = currentTime;
		lastSecondFrameCount = frameCounter;
		frameCounter = 0;
	}

	frameInfo.elapsedTime = duration_cast<Time<float>::Duration>(currentTime - startTime);
	frameInfo.deltaTime = duration_cast<Time<float>::Duration>(deltaTime);

	update(frameInfo);
	if (maxTicksPerFrame > 0) {
		const Clock::duration timeSinceLatestTick = currentTime - latestTickProcessingEndTime;
		for (Clock::rep ticksToProcess = std::min(timeSinceLatestTick / tickInterval, maxTicksPerFrame); ticksToProcess > 0; --ticksToProcess) {
			tick(tickInfo);
			++tickInfo.processedTickCount;
			tickInfo.processedTickTime += tickInfo.tickInterval;
			latestTickProcessingEndTime += tickInterval;
		}
	}

	frameInfo.tickInterpolationAlpha =
		std::min(1.0f, duration_cast<Time<float>::Duration>(currentTime - latestTickProcessingEndTime) / Time<float>::Duration{tickInfo.tickInterval});

	display(tickInfo, frameInfo);
}

} // namespace donut::application
