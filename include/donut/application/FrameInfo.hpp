#ifndef DONUT_APPLICATION_FRAME_INFO_HPP
#define DONUT_APPLICATION_FRAME_INFO_HPP

#include <donut/Time.hpp>

namespace donut::application {

/**
 * Transient information about the current frame of an Application.
 */
struct FrameInfo {
	/**
	 * The ratio of the latest processed tick's importance compared to the tick
	 * processed before it, for use when interpolating data between the two.
	 */
	float tickInterpolationAlpha;

	/**
	 * The time, in seconds, that had elapsed since the start of the application
	 * at the beginning of the current frame.
	 */
	Time<float> elapsedTime;

	/**
	 * The time, in seconds, elapsed between the beginning of the previous frame
	 * and the beginning of the current frame.
	 */
	Time<float> deltaTime;
};

} // namespace donut::application

#endif
