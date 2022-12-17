#ifndef DONUT_AUDIO_SOUND_HPP
#define DONUT_AUDIO_SOUND_HPP

#include <donut/Resource.hpp>

#include <limits> // std::numeric_limits

namespace donut {
namespace audio {

/**
 * Distance attenuation/falloff model for 3D positional audio.
 */
enum class SoundAttenuationModel : unsigned {
	/**
	 * No distance attenuation; sound has the same volume regardless of distance
	 * between the sound instance and the listener.
	 */
	NO_ATTENUATION,

	/**
	 * Attenuate the amplitude of the sound by the inverse distance between the
	 * sound instance and the listener according to the formula:
	 * ```
	 * gain = dmin / (dmin + r * (clamp(d, dmin, dmax) - dmin))
	 * ```
	 * where:
	 * - d is the linear distance between the sound instance and the listener,
	 * - dmin = minDistance,
	 * - dmax = maxDistance,
	 * - r = rolloffFactor.
	 *
	 * \note When using this attenuation model, it is recommended to use a
	 *       rolloffFactor greater than or equal to 1.
	 *
	 * \warning When using this attenuation model:
	 *          - minDistance must be less than or equal to maxDistance.
	 *          - minDistance must be greater than 0.
	 *          - rolloffFactor must be greater than 0.
	 */
	INVERSE_DISTANCE,

	/**
	 * Attenuate the amplitude of the sound by the linear distance between the
	 * sound instance and the listener according to the formula:
	 * ```
	 * gain = 1 - r * (clamp(d, dmin, dmax) - dmin) / (dmax - dmin)
	 * ```
	 * where:
	 * - d is the linear distance between the sound instance and the listener,
	 * - dmin = minDistance,
	 * - dmax = maxDistance,
	 * - r = rolloffFactor.
	 *
	 * \warning When using this attenuation model:
	 *          - minDistance must be less than or equal to maxDistance.
	 *          - rolloffFactor must be between 0 and 1 (inclusive).
	 */
	LINEAR_DISTANCE,

	/**
	 * Attenuate the amplitude of the sound by the exponential distance between
	 * the sound instance and the listener according to the formula:
	 * ```
	 * gain = pow(clamp(d, dmin, dmax) / dmin, -r)
	 * ```
	 * where:
	 * - d is the linear distance between the sound instance and the listener,
	 * - dmin = minDistance,
	 * - dmax = maxDistance,
	 * - r = rolloffFactor.
	 *
	 * \note When using this attenuation model, it is recommended to use a
	 *       rolloffFactor greater than or equal to 1.
	 *
	 * \warning When using this attenuation model:
	 *          - minDistance must be less than or equal to maxDistance.
	 *          - minDistance must be greater than 0.
	 *          - rolloffFactor must be greater than 0.
	 */
	EXPONENTIAL_DISTANCE,
};

/**
 * Configuration options for a sound.
 */
struct SoundOptions {
	/**
	 * Which distance attenuation/falloff model to use for 3D positional audio
	 * when playing this sound. See SoundAttenuationModel for the different
	 * alternatives.
	 *
	 * \remark the recommended model for most applications is
	 *         SoundAttenuationModel::INVERSE_DISTANCE.
	 */
	SoundAttenuationModel attenuationModel = SoundAttenuationModel::INVERSE_DISTANCE;

	/**
	 * The default volume of instances of this sound, which is used if no volume
	 * override is specified when the sound is played.
	 *
	 * When used, the amplitude of the playing sound is multiplied by this gain
	 * value, meaning that a value of 1 represents no change, i.e. 100% of the
	 * original volume of the loaded sound file.
	 */
	float volume = 1.0f;

	/**
	 * The minimum distance of the range where the distance between the sound
	 * instance and listener changes the sound attenuation/falloff for this
	 * sound.
	 *
	 * See SoundAttenuationModel for the effect this parameter has when using
	 * the different attenuation models, as well as the restrictions on its
	 * value for well-defined results.
	 *
	 * \warning When using the default attenuation model, this value must be
	 *          greater than 0, and must also be less than or equal to
	 *          maxDistance.
	 */
	float minDistance = 1.0f;

	/**
	 * The maximum distance of the range where the distance between the sound
	 * instance and listener changes sound attenuation/falloff for this sound.
	 *
	 * Beyond this range, the distance between sound instance and listener stops
	 * having an effect on the volume.
	 *
	 * See SoundAttenuationModel for the effect this parameter has when using
	 * the different attenuation models, as well as the restrictions on its
	 * value for well-defined results.
	 *
	 * \warning When using the default attenuation model, this value must be
	 *          greater than or equal to minDistance.
	 */
	float maxDistance = std::numeric_limits<float>::max();

	/**
	 * The rolloff factor to use in the attenuation/falloff calculation for this
	 * sound.
	 *
	 * In general, a larger rolloff factor causes the sound volume to drop more
	 * steeply with the distance between the sound instance and listener.
	 *
	 * See SoundAttenuationModel for the effect this parameter has when using
	 * the different attenuation models, as well as the restrictions on its
	 * value for well-defined results.
	 *
	 * \note A value greater than or equal to 1 is usually recommended.
	 *
	 * \warning When using the default attenuation model, this value must be
	 *          greater than 0.
	 */
	float rolloffFactor = 1.0f;

	/**
	 * Strength of the doppler effect for this sound.
	 *
	 * The doppler effect depends on the velocity of the sound instance and the
	 * listener as well as the speed of sound that is set in the SoundStage.
	 * When both velocities are 0, there is no doppler effect, and in that case
	 * this parameter makes no difference to the sound.
	 */
	float dopplerFactor = 1.0f;

	/**
	 * Simulate the delay due to the speed of sound between the sound being
	 * played and the sound being heard.
	 *
	 * When enabled, the delay depends on the distance between the sound
	 * instance and the listener as well as the sound speed set in the
	 * SoundStage.
	 */
	bool useDistanceDelay = false;

	/**
	 * Don't take the listener's sound stage position into account when playing
	 * this sound.
	 *
	 * When enabled, the position of the sound instance is treated as being
	 * relative to the listener, as if the listener's position is (0, 0, 0).
	 */
	bool listenerRelative = false;

	/**
	 * Play this sound on repeat instead of just playing it once.
	 */
	bool looping = false;

	/**
	 * Override any instances of this sound that are already playing when a new
	 * instance is played.
	 *
	 * Useful for making sure a certain sound effect never overlaps with itself
	 * when played multiple times.
	 */
	bool singleInstance = false;
};

/**
 * In-memory representation of a particular sound wave that can be played in a
 * SoundStage.
 *
 * A single loaded sound can be used to spawn multiple sound instances that play
 * the same sound at different times or in parallel, and with potentially
 * varying volumes, positions and velocities.
 */
class Sound {
public:
	/**
	 * Load a sound from a virtual file.
	 *
	 * The supported file formats are:
	 * - Vorbis (.ogg)
	 * - RIFF (.wav)
	 * - FLAC (.flac)
	 * - MP3 (.mp3)
	 *
	 * \param filepath virtual filepath of the sound file to load, see
	 *        donut::File.
	 * \param options sound options, see SoundOptions.
	 *
	 * \throw audio::Error on failure to open or load the file.
	 * \throw std::bad_alloc on allocation failure.
	 */
	explicit Sound(const char* filepath, const SoundOptions& options = {});

	/**
	 * Get an opaque handle to the internal representation of the sound.
	 *
	 * \return an untyped pointer to the internal representation of the sound.
	 *
	 * \note This function is used internally by the SoundStage implementation
	 *       and is not intended to be used outside of it. The returned handle
	 *       has no meaning to application code.
	 */
	[[nodiscard]] void* get() const noexcept {
		return buffer.get();
	}

private:
	struct SourceDeleter {
		void operator()(void* handle) const noexcept;
	};

	using Source = Resource<void*, SourceDeleter, nullptr>;

	Source buffer{};
};

} // namespace audio
} // namespace donut

#endif
