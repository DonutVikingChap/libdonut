#ifndef DONUT_TIMER_HPP
#define DONUT_TIMER_HPP

namespace donut {

template <typename Duration>
class Timer {
public:
	constexpr Timer() noexcept = default;

	constexpr explicit Timer(Duration time) noexcept
		: time(time) {}

	constexpr void reset(Duration startTime = {}) noexcept {
		time = startTime;
	}

	constexpr void addTime(Duration deltaTime) noexcept {
		time += deltaTime;
	}

	constexpr void subtractTime(Duration deltaTime) noexcept {
		time -= deltaTime;
	}

	constexpr bool countDown(Duration deltaTime, Duration targetTime) noexcept {
		time -= deltaTime;
		if (time <= targetTime) {
			time = targetTime;
			return true;
		}
		return false;
	}

	constexpr bool countUp(Duration deltaTime, Duration targetTime) noexcept {
		time += deltaTime;
		if (time >= targetTime) {
			time = targetTime;
			return true;
		}
		return false;
	}

	[[nodiscard]] constexpr int countDownLoop(Duration deltaTime, Duration interval) noexcept {
		if (interval <= Duration{}) {
			return 1;
		}
		int ticks = 0;
		time -= deltaTime;
		while (time <= Duration{}) {
			time += interval;
			++ticks;
		}
		return ticks;
	}

	[[nodiscard]] constexpr int countUpLoop(Duration deltaTime, Duration interval) noexcept {
		if (interval <= Duration{}) {
			return 1;
		}
		int ticks = 0;
		time += deltaTime;
		while (time >= interval) {
			time -= interval;
			++ticks;
		}
		return ticks;
	}

	[[nodiscard]] constexpr int countDownLoopTrigger(Duration deltaTime, Duration interval, bool active) noexcept {
		if (active) {
			return countDownLoop(deltaTime, interval);
		}
		countDown(deltaTime, 0.0f);
		return 0;
	}

	[[nodiscard]] constexpr int countUpLoopTrigger(Duration deltaTime, Duration interval, bool active) noexcept {
		if (active) {
			return countUpLoop(deltaTime, interval);
		}
		reset();
		return 0;
	}

	[[nodiscard]] constexpr Duration getTime() const noexcept {
		return time;
	}

private:
	Duration time{};
};

} // namespace donut

#endif
