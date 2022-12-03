#ifndef DONUT_AUDIO_SOUND_STAGE_HPP
#define DONUT_AUDIO_SOUND_STAGE_HPP

#include <donut/Resource.hpp>

#include <glm/glm.hpp>

namespace donut {
namespace audio {

class Sound;

struct SoundStageOptions {
	float volume = 1.0f;
	unsigned maxSimultaneousSounds = 32u;
	glm::vec3 coordinateScale{1.0f, 1.0f, 1.0f};
};

class SoundStage {
public:
	using SoundInstanceId = unsigned;

	explicit SoundStage(const SoundStageOptions& options = {});

	void beginFrame(float deltaTime, glm::vec3 listenerPosition);

	SoundInstanceId playSound(const Sound& sound, float volume = -1.0f, glm::vec3 position = {0.0f, 0.0f, 0.0f}, glm::vec3 velocity = {0.0f, 0.0f, 0.0f});
	SoundInstanceId playSoundInBackground(const Sound& sound, float volume = -1.0f);

	SoundInstanceId createPausedSoundInBackground(const Sound& sound, float volume = -1.0f);

	[[nodiscard]] bool isSoundPaused(SoundInstanceId id) const noexcept;
	[[nodiscard]] bool isSoundStopped(SoundInstanceId id) const noexcept;

	void stopSound(SoundInstanceId id);
	void pauseSound(SoundInstanceId id);
	void resumeSound(SoundInstanceId id);
	void scheduleSoundStop(SoundInstanceId id, float timePointInSound);
	void scheduleSoundPause(SoundInstanceId id, float timePointInSound);
	void seekToSoundTime(SoundInstanceId id, float timePointInSound);

	void setVolume(float volume);
	void setMaxSimultaneousSounds(unsigned maxSimultaneousSounds);

private:
	struct EngineDeleter {
		void operator()(void* handle) const noexcept;
	};

	using Engine = Resource<void*, EngineDeleter, nullptr>;

	Engine engine{};
	glm::vec3 coordinateScale;
	float time = 0.0f;
};

} // namespace audio
} // namespace donut

#endif
