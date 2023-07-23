#ifndef DONUT_APPLICATION_TICK_INFO_HPP
#define DONUT_APPLICATION_TICK_INFO_HPP

#include <donut/Time.hpp>

#include <cstddef> // std::size_t

namespace donut::application {

/**
 * Transient information about the current tick of an Application.
 */
struct TickInfo {
	/**
	 * The average time, in seconds, that should elapse between each tick.
	 *
	 * This is calculated as the reciprocal of the desired application tick
	 * rate, i.e. tickInterval = 1 / ApplicationOptions::tickRate.
	 *
	 * The tick interval should be used as the time delta when updating
	 * physics, timers, etc. within a tick. This will ensure a fixed update
	 * interval, which generally results in more stable, predictable and
	 * consistent behavior compared to using a variable update interval,
	 * especially with regard to floating-point errors and numerical integration
	 * methods which may produce different results depending on the step size.
	 *
	 * To achieve a higher perceived update rate for the user, some form of
	 * interpolation and/or extrapolation should be used in the variable-rate
	 * Application::display() callback in order to smooth out the result of the
	 * fixed-rate ticks when applicable.
	 */
	Time<float> tickInterval;

	/**
	 * Number of ticks that have been fully processed since the start of the
	 * application.
	 */
	std::size_t processedTickCount;

	/**
	 * The accumulated time, in seconds, of all ticks that had been processed
	 * since the start of the application at the beginning of the current tick.
	 */
	Time<float> processedTickTime;
};

} // namespace donut::application

#endif
