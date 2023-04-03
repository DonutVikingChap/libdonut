#ifndef DONUT_TIMER_HPP
#define DONUT_TIMER_HPP

namespace donut {

/**
 * Time counting utility for variable-rate polling of fixed-rate events.
 *
 * \tparam Duration duration type to use for the internal time accumulator.
 */
template <typename Duration>
class Timer {
public:
	/**
	 * Construct a timer with its internal time accumulator set to 0.
	 */
	constexpr Timer() noexcept = default;

	/**
	 * Construct a timer with its internal time accumulator set to a given
	 * value.
	 *
	 * \param time the time value to initialize the accumulator to.
	 */
	constexpr explicit Timer(Duration time) noexcept
		: time(time) {}

	/**
	 * Set the internal time accumulator to a given value.
	 *
	 * \param startTime the new time value to set the accumulator to.
	 *
	 * \sa addTime()
	 * \sa subtractTime()
	 */
	constexpr void reset(Duration startTime = {}) noexcept {
		time = startTime;
	}

	/**
	 * Add time to the internal time accumulator.
	 *
	 * \param deltaTime the time delta to add to the accumulator.
	 *
	 * \sa reset()
	 * \sa subtractTime()
	 */
	constexpr void addTime(Duration deltaTime) noexcept {
		time += deltaTime;
	}

	/**
	 * Subtract time from the internal time accumulator.
	 *
	 * \param deltaTime the time delta to subtract from the accumulator.
	 *
	 * \sa reset()
	 * \sa addTime()
	 */
	constexpr void subtractTime(Duration deltaTime) noexcept {
		time -= deltaTime;
	}

	/**
	 * Subtract time from the internal time accumulator and check if it has
	 * reached the given target time.
	 *
	 * \param deltaTime the time delta to subtract from the accumulator.
	 * \param targetTime target time.
	 *
	 * \return true if the internal time accumulator has reached below or equal
	 *         to the target time, false otherwise.
	 *
	 * \note The time accumulator is clamped to the target time when the target
	 *       is reached.
	 *
	 * \sa countUp()
	 * \sa countDownLoop()
	 */
	constexpr bool countDown(Duration deltaTime, Duration targetTime = {}) noexcept {
		time -= deltaTime;
		if (time <= targetTime) {
			time = targetTime;
			return true;
		}
		return false;
	}

	/**
	 * Add time to the internal time accumulator and check if it has reached the
	 * given target time.
	 *
	 * \param deltaTime the time delta to add to the accumulator.
	 * \param targetTime target time.
	 *
	 * \return true if the internal time accumulator has reached above or equal
	 *         to the target time, false otherwise.
	 *
	 * \note The time accumulator is clamped to the target time when the target
	 *       is reached.
	 *
	 * \sa countDown()
	 * \sa countUpLoop()
	 */
	constexpr bool countUp(Duration deltaTime, Duration targetTime) noexcept {
		time += deltaTime;
		if (time >= targetTime) {
			time = targetTime;
			return true;
		}
		return false;
	}

	/**
	 * Subtract time from the internal time accumulator and check how many times
	 * it has reached 0 while looping back to the given time interval.
	 *
	 * \param deltaTime the time delta to subtract from the accumulator.
	 * \param interval loop interval duration.
	 *
	 * \return the number of times that the internal time accumulator reached
	 *         below or equal to 0. This may be any non-negative integer,
	 *         including 0.
	 *
	 * \note An interval duration of 0 results in the number 1 being returned on
	 *       every call.
	 *
	 * \sa countDown()
	 * \sa countUpLoop()
	 * \sa countDownLoopTrigger()
	 */
	[[nodiscard]] constexpr unsigned countDownLoop(Duration deltaTime, Duration interval) noexcept {
		if (interval <= Duration{}) {
			time = Duration{};
			return 1u;
		}
		unsigned ticks = 0u;
		time -= deltaTime;
		while (time <= Duration{}) {
			time += interval;
			++ticks;
		}
		return ticks;
	}

	/**
	 * Add time to the internal time accumulator and check how many times it has
	 * reached the given interval time while looping back to 0.
	 *
	 * \param deltaTime the time delta to add to the accumulator.
	 * \param interval loop interval duration.
	 *
	 * \return the number of times that the internal time accumulator reached
	 *         above or equal to the interval time. This may be any non-negative
	 *         integer, including 0.
	 *
	 * \note An interval duration of 0 results in the number 1 being returned on
	 *       every call.
	 *
	 * \sa countUp()
	 * \sa countDownLoop()
	 * \sa countUpLoopTrigger()
	 */
	[[nodiscard]] constexpr unsigned countUpLoop(Duration deltaTime, Duration interval) noexcept {
		if (interval <= Duration{}) {
			time = Duration{};
			return 1u;
		}
		unsigned ticks = 0u;
		time += deltaTime;
		while (time >= interval) {
			time -= interval;
			++ticks;
		}
		return ticks;
	}

	/**
	 * Update a countdown loop with a boolean trigger that determines whether
	 * the loop is active or not.
	 *
	 * An inactive loop will continue counting down to 0 but will not loop back
	 * around to the interval time and will always return 0.
	 *
	 * \param deltaTime the time delta to subtract from the accumulator.
	 * \param interval loop interval duration.
	 * \param active whether the loop is currently active or not.
	 *
	 * \return the number of times that the internal time accumulator reached
	 *         below or equal to 0 while active. This may be any non-negative
	 *         integer, including 0.
	 *
	 * \note An interval duration of 0 results in the number 1 being returned on
	 *       every call when the loop is active.
	 *
	 * \remark This function can be used to simulate something like the trigger
	 *         mechanism of a fully automatic firearm firing from a closed bolt,
	 *         since it will fire once as soon as it is activated and then keep
	 *         firing at a fixed cyclic rate until the trigger is released, at
	 *         which point the mechanism will continue to cycle into the closed
	 *         position even if the trigger is not held, where it will then stop
	 *         without firing the next round, and be ready to fire immediately
	 *         when the trigger is activated again. Reactivating the trigger
	 *         before the mechanism has fully cycled does not make it fire more
	 *         quickly.
	 *
	 * \sa countDownLoop()
	 * \sa countUpLoopTrigger()
	 */
	[[nodiscard]] constexpr unsigned countDownLoopTrigger(Duration deltaTime, Duration interval, bool active) noexcept {
		if (active) {
			return countDownLoop(deltaTime, interval);
		}
		countDown(deltaTime, 0.0f);
		return 0u;
	}

	/**
	 * Update a countup loop with a boolean trigger that determines whether the
	 * loop is active or not.
	 *
	 * An inactive loop will reset itself to 0 and will always return 0.
	 *
	 * \param deltaTime the time delta to add to the accumulator.
	 * \param interval loop interval duration.
	 * \param active whether the loop is currently active or not.
	 *
	 * \return the number of times that the internal time accumulator reached
	 *         above or equal to the interval time while active. This may be any
	 *         non-negative integer, including 0.
	 *
	 * \note An interval duration of 0 results in the number 1 being returned on
	 *       every call when the loop is active.
	 *
	 * \remark This function can be used to simulate something like the trigger
	 *         mechanism of a hypothetical "railgun" that needs to be fully
	 *         charged before it can fire. Activating the trigger starts the
	 *         charging process, which can be canceled at any time by
	 *         deactivating the trigger, which immediately resets the charge
	 *         back to 0. The mechanism fires as soon as it is fully charged and
	 *         then immediately starts charging the next round if the trigger is
	 *         still held.
	 *
	 * \sa countUpLoop()
	 * \sa countDownLoopTrigger()
	 */
	[[nodiscard]] constexpr unsigned countUpLoopTrigger(Duration deltaTime, Duration interval, bool active) noexcept {
		if (active) {
			return countUpLoop(deltaTime, interval);
		}
		reset();
		return 0u;
	}

	/**
	 * Get the time value of the internal accumulator.
	 *
	 * \return a copy of the current time value.
	 */
	[[nodiscard]] constexpr Duration getTime() const noexcept {
		return time;
	}

private:
	Duration time{};
};

} // namespace donut

#endif
