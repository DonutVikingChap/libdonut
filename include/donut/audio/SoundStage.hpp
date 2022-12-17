#ifndef DONUT_AUDIO_SOUND_STAGE_HPP
#define DONUT_AUDIO_SOUND_STAGE_HPP

#include <donut/Resource.hpp>

#include <glm/glm.hpp> // glm::...

namespace donut {
namespace audio {

class Sound;

/**
 * Current state of the sound listener, i.e. the user perceiving the audio,
 * within a SoundStage.
 *
 * This information is used in the calculations for various 3D sound effect
 * simulations such as distance attenuation, sound delay and the doppler effect.
 */
struct SoundListener {
	/**
	 * Position of the listener in the sound stage.
	 *
	 * Used for distance attenuation and sound delay, if enabled.
	 */
	glm::vec3 position{0.0f, 0.0f, 0.0f};

	/**
	 * Linear velocity of the listener.
	 *
	 * Used in doppler effect calculations.
	 */
	glm::vec3 velocity{0.0f, 0.0f, 0.0f};

	/**
	 * The direction the listener is facing.
	 *
	 * Does not need to be normalized.
	 */
	glm::vec3 aimDirection{0.0f, 0.0f, -1.0f};

	/**
	 * The direction upwards from the listener.
	 *
	 * Does not need to be normalized.
	 */
	glm::vec3 up{0.0f, 1.0f, 0.0f};
};

/**
 * Configuration options for a SoundStage.
 */
struct SoundStageOptions {
	/**
	 * Global master volume.
	 *
	 * The amplitude of all playing sound is multiplied by this gain value,
	 * meaning that a value of 1 represents no change, i.e. 100% of the original
	 * volume.
	 */
	float volume = 1.0f;

	/**
	 * The speed of sound in the sound stage.
	 *
	 * This value is used for doppler effect calculations and distance delay
	 * simulation.
	 *
	 * \note The default value assumes that any coordinates passed to the sound
	 *       stage are expressed in meters, and that the sound stage environment
	 *       is dry air at around 20 degrees Celsius.
	 */
	float soundSpeed = 343.3f;

	/**
	 * The maximum total number of sound instances that can play simultaneously.
	 * If the number of playing sounds exceeds this number, the ones with the
	 * highest volume will be picked to actually play.
	 */
	unsigned maxSimultaneousSounds = 32u;
};

/**
 * Persistent system for playing sound in a simulated 3D arena to the default
 * audio device.
 *
 * The sound stage uses a right-handed coordinate system for 3D calculations,
 * and any coordinates are assumed to be in meters by default. Applications that
 * use a different unit of length should adjust the
 * SoundStageOptions::soundSpeed in the sound stage configuration as well as the
 * SoundOptions::rolloffFactor of each Sound accordingly.
 */
class SoundStage {
public:
	/**
	 * Opaque handle to a specific instance of a sound in the sound stage.
	 */
	using SoundInstanceId = unsigned;

	/**
	 * Construct a sound stage.
	 *
	 * \param options initial configuration of the sound stage, see
	 *        SoundStageOptions.
	 *
	 * \throw audio::Error on failure to initialize the underlying audio engine.
	 * \throw std::bad_alloc on allocation failure.
	 *
	 * \warning Due to having global access to the default audio device, only a
	 *          single SoundManager instance may exist in the program at any
	 *          given time.
	 */
	explicit SoundStage(const SoundStageOptions& options = {});

	/**
	 * Update the 3D parameters of the sound stage and advance the current time.
	 *
	 * \param deltaTime time delta to advance the time by, in seconds, typically
	 *        received from the current application::FrameInfo in
	 *        application::Application::update().
	 * \param listener current parameters to use for the sound listener on this
	 *        frame.
	 *
	 * \note This function should typically be called once every frame during
	 *       the application::Application::update() callback.
	 */
	void update(float deltaTime, const SoundListener& listener);

	/**
	 * Create a new 3D sound instance and start playing it.
	 *
	 * \param sound the sound data to use for this instance.
	 * \param volume gain to multiply the amplitude of the sound by when
	 *        playing. If negative, the default volume set in the SoundOptions
	 *        is used instead.
	 * \param position the position to play the sound at, in sound stage
	 *        coordinates. Used for distance attenuation/falloff according to
	 *        the SoundAttenuationModel set in the sound's SoundOptions.
	 * \param velocity initial linear velocity of the sound. Relevant for
	 *        simulating the doppler effect.
	 *
	 * \return a handle to the new sound instance that can be used to refer back
	 *         to it later in order to query its state or make changes to it.
	 *
	 * \sa playSoundInBackground()
	 * \sa createPausedSoundInBackground()
	 */
	SoundInstanceId playSound(const Sound& sound, float volume = -1.0f, glm::vec3 position = {0.0f, 0.0f, 0.0f}, glm::vec3 velocity = {0.0f, 0.0f, 0.0f});

	/**
	 * Create a new sound instance without any panning and start playing it.
	 *
	 * \param sound the sound data to use for this instance.
	 * \param volume gain to multiply the amplitude of the sound by when
	 *        playing. If negative, the default volume set in the SoundOptions
	 *        is used instead.
	 *
	 * \return a handle to the new sound instance that can be used to refer back
	 *         to it later in order to query its state or make changes to it.
	 *
	 * \sa playSound()
	 * \sa createPausedSoundInBackground()
	 */
	SoundInstanceId playSoundInBackground(const Sound& sound, float volume = -1.0f);

	/**
	 * Create a new sound instance without any panning, but don't start playing
	 * it yet.
	 *
	 * \param sound the sound data to use for this instance.
	 * \param volume gain to multiply the amplitude of the sound by when
	 *        playing. If negative, the default volume set in the SoundOptions
	 *        is used instead.
	 *
	 * \return a handle to the new sound instance that can be used to refer back
	 *         to it later in order to query its state or make changes to it.
	 *
	 * \sa playSound()
	 * \sa playSoundInBackground()
	 */
	SoundInstanceId createPausedSoundInBackground(const Sound& sound, float volume = -1.0f);

	/**
	 * Check if a specific sound instance is currently paused.
	 *
	 * \param id handle to the sound instance, acquired when it was created.
	 *
	 * \return true if the sound instance still exists and is currently paused,
	 *         false otherwise.
	 *
	 * \sa isSoundStopped()
	 * \sa pauseSound()
	 * \sa resumeSound()
	 */
	[[nodiscard]] bool isSoundPaused(SoundInstanceId id) const noexcept;

	/**
	 * Check if a specific sound instance has finished playing.
	 *
	 * \param id handle to the sound instance, acquired when it was created.
	 *
	 * \return true if the sound instance has been stopped or finished playing
	 *         and no longer exists, false otherwise.
	 *
	 * \sa isSoundPaused()
	 * \sa stopSound()
	 */
	[[nodiscard]] bool isSoundStopped(SoundInstanceId id) const noexcept;

	/**
	 * Stop a specific sound instance and remove it.
	 *
	 * \param id handle to the sound instance, acquired when it was created.
	 *
	 * \note if the given sound instance doesn't exist, this function has no
	 *       effect.
	 *
	 * \sa isSoundStopped()
	 * \sa scheduleSoundStop()
	 */
	void stopSound(SoundInstanceId id);

	/**
	 * Pause a specific sound instance.
	 *
	 * \param id handle to the sound instance, acquired when it was created.
	 *
	 * \note if the given sound instance doesn't exist, or if it is already
	 *       paused, this function has no effect.
	 *
	 * \sa isSoundPaused()
	 * \sa resumeSound()
	 * \sa scheduleSoundPause()
	 */
	void pauseSound(SoundInstanceId id);

	/**
	 * Unpause and resume a specific sound instance.
	 *
	 * \param id handle to the sound instance, acquired when it was created.
	 *
	 * \note if the given sound instance doesn't exist, or if it is not paused,
	 *       this function has no effect.
	 *
	 * \sa isSoundPaused()
	 * \sa pauseSound()
	 */
	void resumeSound(SoundInstanceId id);

	/**
	 * Schedule for a specific sound instance to stop playing and remove itself
	 * when the playback reaches a specific time point.
	 *
	 * \param id handle to the sound instance, acquired when it was created.
	 * \param timePointInSound the time point, measured in seconds from the
	 *        beginning of the sound, where the sound instance will stop itself.
	 *
	 * \note if the given sound instance doesn't exist, this function has no
	 *       effect.
	 *
	 * \sa isSoundStopped()
	 * \sa stopSound()
	 */
	void scheduleSoundStop(SoundInstanceId id, float timePointInSound);

	/**
	 * Schedule for a specific sound instance to pause itself when the playback
	 * reaches a specific time point.
	 *
	 * \param id handle to the sound instance, acquired when it was created.
	 * \param timePointInSound the time point, measured in seconds from the
	 *        beginning of the sound, where the sound instance will pause
	 *        itself.
	 *
	 * \note if the given sound instance doesn't exist, this function has no
	 *       effect.
	 *
	 * \sa isSoundPaused()
	 * \sa pauseSound()
	 */
	void scheduleSoundPause(SoundInstanceId id, float timePointInSound);

	/**
	 * Set the current playback time point for a specific sound instance.
	 *
	 * \param id handle to the sound instance, acquired when it was created.
	 * \param timePointInSound the time point, measured in seconds from the
	 *        beginning of the sound, to seek to.
	 *
	 * \note if the given sound instance exists, it will continue playing after
	 *       seeking unless it was paused.
	 * \note if the given sound instance doesn't exist, this function has no
	 *       effect.
	 */
	void seekToSoundTime(SoundInstanceId id, float timePointInSound);

	/**
	 * Set the current 3D position of a specific sound instance.
	 *
	 * \param id handle to the sound instance, acquired when it was created.
	 * \param newPosition new position of the sound instance, in sound stage
	 *        coordinates.
	 *
	 * \note the effect of this function will only apply after the next call to
	 *       update().
	 * \note if the given sound instance doesn't exist, this function has no
	 *       effect.
	 *
	 * \sa setSoundVelocity()
	 * \sa setSoundPositionAndVelocity()
	 */
	void setSoundPosition(SoundInstanceId id, glm::vec3 newPosition);

	/**
	 * Set the current 3D velocity of a specific sound instance.
	 *
	 * \param id handle to the sound instance, acquired when it was created.
	 * \param newVelocity new linear velocity of the sound instance.
	 *
	 * \note the effect of this function will only apply after the next call to
	 *       update().
	 * \note if the given sound instance doesn't exist, this function has no
	 *       effect.
	 *
	 * \sa setSoundPosition()
	 * \sa setSoundPositionAndVelocity()
	 */
	void setSoundVelocity(SoundInstanceId id, glm::vec3 newVelocity);

	/**
	 * Set both the 3D position and 3D velocity of a specific sound instance at
	 * the same time.
	 *
	 * \param id handle to the sound instance, acquired when it was created.
	 * \param newPosition new position of the sound instance, in sound stage
	 *        coordinates.
	 * \param newVelocity new linear velocity of the sound instance.
	 *
	 * \note the effect of this function will only apply after the next call to
	 *       update().
	 * \note if the given sound instance doesn't exist, this function has no
	 *       effect.
	 *
	 * \sa setSoundPosition()
	 * \sa setSoundVelocity()
	 */
	void setSoundPositionAndVelocity(SoundInstanceId id, glm::vec3 newPosition, glm::vec3 newVelocity);

	/**
	 * Set the global master volume of the sound stage.
	 *
	 * \param volume new volume, see SoundStageOptions::volume.
	 */
	void setVolume(float volume);

	/**
	 * Set the speed of sound in the sound stage.
	 *
	 * \param soundSpeed new speed of sound, see SoundStageOptions::soundSpeed.
	 */
	void setSoundSpeed(float soundSpeed);

	/**
	 * Set the maximum total number of sound instances that can play
	 * simultaneously.
	 *
	 * \param maxSimultaneousSounds new number of sound instances, see
	 *        SoundStageOptions::maxSimultaneousSounds.
	 */
	void setMaxSimultaneousSounds(unsigned maxSimultaneousSounds);

private:
	struct EngineDeleter {
		void operator()(void* handle) const noexcept;
	};

	using Engine = Resource<void*, EngineDeleter, nullptr>;

	Engine engine{};
	float time = 0.0f;
};

} // namespace audio
} // namespace donut

#endif
