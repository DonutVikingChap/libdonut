#ifndef DONUT_AUDIO_SOUND_HPP
#define DONUT_AUDIO_SOUND_HPP

#include <donut/Resource.hpp>

#include <limits> // std::numeric_limits

namespace donut {
namespace audio {

enum class SoundAttenuationModel : unsigned {
	NO_ATTENUATION,
	INVERSE_DISTANCE,
	LINEAR_DISTANCE,
	EXPONENTIAL_DISTANCE,
};

struct SoundOptions {
	SoundAttenuationModel attenuationModel = SoundAttenuationModel::INVERSE_DISTANCE;
	float volume = 1.0f;
	float minDistance = 1.0f;
	float maxDistance = std::numeric_limits<float>::max();
	float rolloffFactor = 1.0f;
	float dopplerFactor = 0.0f;
	bool useDistanceDelay = false;
	bool listenerRelative = false;
	bool looping = false;
};

class Sound {
public:
	explicit Sound(const char* filepath, const SoundOptions& options = {});

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
