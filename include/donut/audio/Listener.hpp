#ifndef DONUT_AUDIO_LISTENER_HPP
#define DONUT_AUDIO_LISTENER_HPP

#include <donut/math.hpp>

namespace donut::audio {

/**
 * Current state of the sound listener, i.e. the user perceiving the audio,
 * within a SoundStage.
 *
 * This information is used in the calculations for various 3D sound effect
 * simulations such as distance attenuation, sound delay and the doppler effect.
 */
struct Listener {
	/**
	 * Position of the listener in the sound stage.
	 *
	 * Used for distance attenuation and sound delay, if enabled.
	 */
	vec3 position{0.0f, 0.0f, 0.0f};

	/**
	 * Linear velocity of the listener.
	 *
	 * Used in doppler effect calculations.
	 */
	vec3 velocity{0.0f, 0.0f, 0.0f};

	/**
	 * The direction the listener is facing.
	 *
	 * Does not need to be normalized.
	 */
	vec3 aimDirection{0.0f, 0.0f, -1.0f};

	/**
	 * The direction upwards from the listener.
	 *
	 * Does not need to be normalized.
	 */
	vec3 up{0.0f, 1.0f, 0.0f};
};

} // namespace donut::audio

#endif
