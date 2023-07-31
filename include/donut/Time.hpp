#ifndef DONUT_TIME_HPP
#define DONUT_TIME_HPP

#include <donut/math.hpp>

#include <chrono>  // std::chrono::...
#include <cstddef> // std::size_t
#include <format>  // std::formatter, std::format_to
#include <ratio>   // std::ratio

namespace donut {

/**
 * Time duration value wrapper.
 *
 * \tparam T scalar type to use as the representation for the underlying
 *         duration.
 * \tparam Period period corresponding to the value 1 of the underlying time
 *         representation type, measured in seconds, expressed as a standard
 *         ratio type.
 */
template <typename T, typename Period = std::ratio<1>>
class Time {
public:
	/**
	 * Underlying duration type.
	 */
	using Duration = std::chrono::duration<T, Period>;

	/**
	 * Construct a time value of 0.
	 */
	constexpr Time() noexcept = default;

	/**
	 * Construct a time value from a duration.
	 *
	 * \param duration the duration to initialize the time value to.
	 */
	constexpr Time(Duration duration) noexcept
		: duration(duration) {}

	/**
	 * Construct a time value from a number of seconds.
	 *
	 * \param seconds the number of seconds to initialize the time value to.
	 */
	constexpr Time(std::chrono::seconds seconds) noexcept
		: duration(duration_cast<Duration>(seconds)) {}

	/**
	 * Construct a time value from a number of milliseconds.
	 *
	 * \param milliseconds the number of milliseconds to initialize the time
	 *        value to.
	 */
	constexpr Time(std::chrono::milliseconds milliseconds) noexcept
		: duration(duration_cast<Duration>(milliseconds)) {}

	/**
	 * Construct a time value from a number of microseconds.
	 *
	 * \param microseconds the number of microseconds to initialize the time
	 *        value to.
	 */
	constexpr Time(std::chrono::microseconds microseconds) noexcept
		: duration(duration_cast<Duration>(microseconds)) {}

	/**
	 * Construct a time value from a number of nanoseconds.
	 *
	 * \param nanoseconds the number of microseconds to initialize the time
	 *        value to.
	 */
	constexpr Time(std::chrono::nanoseconds nanoseconds) noexcept
		: duration(duration_cast<Duration>(nanoseconds)) {}

	/**
	 * Construct a time value from a number of minutes.
	 *
	 * \param minutes the number of minutes to initialize the time value to.
	 */
	constexpr Time(std::chrono::minutes minutes) noexcept
		: duration(duration_cast<Duration>(minutes)) {}

	/**
	 * Construct a time value from a number of hours.
	 *
	 * \param hours the number of hours to initialize the time value to.
	 */
	constexpr Time(std::chrono::hours hours) noexcept
		: duration(duration_cast<Duration>(hours)) {}

	/**
	 * Convert a time value to its underlying duration type.
	 *
	 * \return the underlying duration value.
	 */
	constexpr operator Duration() const noexcept {
		return duration;
	}

	/**
	 * Convert a time value to its underlying scalar type.
	 *
	 * \return the scalar value of the underlying duration's representation.
	 */
	constexpr operator T() const noexcept {
		return duration.count();
	}

	/**
	 * Compare two time values for equality.
	 *
	 * \param a first time value.
	 * \param b second time value.
	 *
	 * \return true if the first and second time values are equal, false
	 *         otherwise.
	 */
	[[nodiscard]] friend constexpr bool operator==(const Time& a, const Time& b) noexcept {
		return a.duration == b.duration;
	}

	/**
	 * Compare two time values for inequality.
	 *
	 * \param a first time value.
	 * \param b second time value.
	 *
	 * \return true if the first and second time values are not equal, false
	 *         otherwise.
	 */
	[[nodiscard]] friend constexpr bool operator!=(const Time& a, const Time& b) noexcept {
		return a.duration != b.duration;
	}

	/**
	 * Check if a time value is less than another time value.
	 *
	 * \param a first time value.
	 * \param b second time value.
	 *
	 * \return true if the first time value is less than the second time value,
	 *         false otherwise.
	 */
	[[nodiscard]] friend constexpr bool operator<(const Time& a, const Time& b) noexcept {
		return a.duration < b.duration;
	}

	/**
	 * Check if a time value is less than or equal to another time value.
	 *
	 * \param a first time value.
	 * \param b second time value.
	 *
	 * \return true if the first time value is less than or equal to the second
	 *         time value, false otherwise.
	 */
	[[nodiscard]] friend constexpr bool operator<=(const Time& a, const Time& b) noexcept {
		return a.duration <= b.duration;
	}

	/**
	 * Check if a time value is greater than another time value.
	 *
	 * \param a first time value.
	 * \param b second time value.
	 *
	 * \return true if the first time value is greater than the second time
	 *         value, false otherwise.
	 */
	[[nodiscard]] friend constexpr bool operator>(const Time& a, const Time& b) noexcept {
		return a.duration > b.duration;
	}

	/**
	 * Check if a time value is greater than or equal to another time value.
	 *
	 * \param a first time value.
	 * \param b second time value.
	 *
	 * \return true if the first time value is greater than or equal to the
	 *         second time value, false otherwise.
	 */
	[[nodiscard]] friend constexpr bool operator>=(const Time& a, const Time& b) noexcept {
		return a.duration >= b.duration;
	}

	/**
	 * Add time to this time value.
	 *
	 * \param deltaTime the time delta to add to this value.
	 *
	 * \return `*this`, for chaining.
	 */
	constexpr Time& operator+=(Time deltaTime) noexcept {
		duration += deltaTime;
		return *this;
	}

	/**
	 * Subtract time from this time value.
	 *
	 * \param deltaTime the time delta to subtract from this value.
	 *
	 * \return `*this`, for chaining.
	 */
	constexpr Time& operator-=(Time deltaTime) noexcept {
		duration -= deltaTime;
		return *this;
	}

	/**
	 * Add two time values.
	 *
	 * \param a first time value.
	 * \param b second time value.
	 *
	 * \return the sum of the first and second time values.
	 */
	friend constexpr Time operator+(Time a, Time b) noexcept {
		a += b;
		return a;
	}

	/**
	 * Subtract a time value from another.
	 *
	 * \param a first time value.
	 * \param b second time value.
	 *
	 * \return the difference between the first and second time values.
	 */
	friend constexpr Time operator-(Time a, Time b) noexcept {
		a -= b;
		return a;
	}

	/**
	 * Subtract time from this value and then check if it reached a given target
	 * time value.
	 *
	 * \param deltaTime the time delta to subtract from this value.
	 * \param targetTime target time.
	 *
	 * \return true if the time value reached below or equal to the target time,
	 *         false otherwise.
	 *
	 * \note The time value is clamped to the target time when the target is
	 *       reached.
	 *
	 * \sa countUp()
	 * \sa countDownLoop()
	 */
	constexpr bool countDown(Time deltaTime, Time targetTime = Duration{}) noexcept {
		duration -= deltaTime;
		if (duration <= targetTime) {
			duration = targetTime;
			return true;
		}
		return false;
	}

	/**
	 * Add time to this value and then check if it reached a given target time
	 * value.
	 *
	 * \param deltaTime the time delta to add to this value.
	 * \param targetTime target time.
	 *
	 * \return true if the time value reached above or equal to the target time,
	 *         false otherwise.
	 *
	 * \note The time value is clamped to the target time when the target is
	 *       reached.
	 *
	 * \sa countDown()
	 * \sa countUpLoop()
	 */
	constexpr bool countUp(Time deltaTime, Time targetTime) noexcept {
		duration += deltaTime;
		if (duration >= targetTime) {
			duration = targetTime;
			return true;
		}
		return false;
	}

	/**
	 * Subtract time from this value and then check how many times it reached 0
	 * while looping back to a given time interval.
	 *
	 * \param deltaTime the time delta to subtract from this value.
	 * \param interval loop interval duration.
	 *
	 * \return the number of times that the time value reached below or equal to
	 *         0 and looped back around. This may be any non-negative integer,
	 *         including 0.
	 *
	 * \note An interval duration of 0 results in the number 1 being returned
	 *       every time.
	 *
	 * \sa countDown()
	 * \sa countUpLoop()
	 * \sa countDownLoopTrigger()
	 */
	[[nodiscard]] constexpr std::size_t countDownLoop(Time deltaTime, Time interval) noexcept {
		if (interval <= Time{}) {
			duration = Time{};
			return 1;
		}
		std::size_t ticks = 0;
		duration -= deltaTime;
		while (duration <= Time{}) {
			duration += interval;
			++ticks;
		}
		return ticks;
	}

	/**
	 * Add time to this value and then check how many times it reached a given
	 * time interval while looping back to 0.
	 *
	 * \param deltaTime the time delta to add to this value.
	 * \param interval loop interval duration.
	 *
	 * \return the number of times that the time value reached above or equal to
	 *         the interval time and looped back around. This may be any
	 *         non-negative integer, including 0.
	 *
	 * \note An interval duration of 0 results in the number 1 being returned
	 *       every time.
	 *
	 * \sa countUp()
	 * \sa countDownLoop()
	 * \sa countUpLoopTrigger()
	 */
	[[nodiscard]] constexpr std::size_t countUpLoop(Time deltaTime, Time interval) noexcept {
		if (interval <= Time{}) {
			duration = Time{};
			return 1;
		}
		std::size_t ticks = 0;
		duration += deltaTime;
		while (duration >= interval) {
			duration -= interval;
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
	 * \param deltaTime the time delta to subtract from this value.
	 * \param interval loop interval duration.
	 * \param active whether the loop is currently active or not.
	 *
	 * \return the number of times that the time value reached below or equal to
	 *         0 while active. This may be any non-negative integer, including
	 *         0.
	 *
	 * \note An interval duration of 0 results in the number 1 being returned
	 *       every time when the loop is active.
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
	[[nodiscard]] constexpr std::size_t countDownLoop(Time deltaTime, Time interval, bool active) noexcept {
		if (active) {
			return countDownLoop(deltaTime, interval);
		}
		countDown(deltaTime);
		return 0;
	}

	/**
	 * Update a countup loop with a boolean trigger that determines whether the
	 * loop is active or not.
	 *
	 * An inactive loop will reset itself to 0 and will always return 0.
	 *
	 * \param deltaTime the time delta to add to this value.
	 * \param interval loop interval duration.
	 * \param active whether the loop is currently active or not.
	 *
	 * \return the number of times that the time value reached above or equal to
	 *         the interval time while active. This may be any non-negative
	 *         integer, including 0.
	 *
	 * \note An interval duration of 0 results in the number 1 being returned
	 *       every time when the loop is active.
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
	[[nodiscard]] constexpr std::size_t countUpLoop(Time deltaTime, Time interval, bool active) noexcept {
		if (active) {
			return countUpLoop(deltaTime, interval);
		}
		*this = Time{};
		return 0;
	}

private:
	Duration duration{};
};

template <length_t L, typename T, typename Period>
[[nodiscard]] constexpr Time<T, Period> operator*(Time<T, Period> a, T b) noexcept {
	return static_cast<typename Time<T, Period>::Duration>(a) * b;
}

template <length_t L, typename T, typename Period>
[[nodiscard]] constexpr Time<T, Period> operator*(T a, Time<T, Period> b) noexcept {
	return a * static_cast<typename Time<T, Period>::Duration>(b);
}

template <length_t L, typename T, typename Period>
[[nodiscard]] constexpr T operator/(Time<T, Period> a, T b) noexcept {
	return static_cast<typename Time<T, Period>::Duration>(a) / b;
}

template <length_t L, typename T, typename Period>
[[nodiscard]] constexpr vec<L, T> operator*(vec<L, T> a, Time<T, Period> b) noexcept {
	return a * static_cast<T>(b);
}

template <length_t L, typename T, typename Period>
[[nodiscard]] constexpr vec<L, T> operator*(Time<T, Period> a, vec<L, T> b) noexcept {
	return static_cast<T>(a) * b;
}

template <length_t L, typename T, typename Period>
[[nodiscard]] constexpr vec<L, T> operator/(vec<L, T> a, Time<T, Period> b) noexcept {
	return a / static_cast<T>(b);
}

template <length_t L, typename T, typename Period>
[[nodiscard]] constexpr vec<L, T> operator/(Time<T, Period> a, vec<L, T> b) noexcept {
	return static_cast<T>(a) / b;
}

} // namespace donut

template <typename T, typename Period, typename CharT>
struct std::formatter<donut::Time<T, Period>, CharT> {
	constexpr auto parse(auto& pc) {
		return pc.begin();
	}

	auto format(const donut::Time<T, Period>& time, auto& fc) const {
		return std::format_to(fc.out(), "{}", static_cast<typename donut::Time<T, Period>::Duration>(time));
	}
};

#endif
