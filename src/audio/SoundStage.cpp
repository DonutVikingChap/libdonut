#include <donut/audio/Error.hpp>
#include <donut/audio/Sound.hpp>
#include <donut/audio/SoundStage.hpp>

#include <glm/glm.hpp>
#include <soloud.h>
#include <soloud_wav.h>

namespace donut {
namespace audio {

SoundStage::SoundStage(const SoundStageOptions& options)
	: engine(new SoLoud::Soloud{})
	, coordinateScale(options.coordinateScale) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	if (const SoLoud::result errorCode = soloud.init(); errorCode != SoLoud::SO_NO_ERROR) {
		throw Error{"Failed to initialize sound manager", errorCode};
	}
	setVolume(options.volume);
	setMaxSimultaneousSounds(options.maxSimultaneousSounds);
}

void SoundStage::beginFrame(float deltaTime, glm::vec3 listenerPosition) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());

	time += deltaTime;

	listenerPosition *= coordinateScale;
	soloud.set3dListenerPosition(listenerPosition.x, listenerPosition.y, listenerPosition.z);

	soloud.update3dAudio();
}

SoundStage::SoundInstanceId SoundStage::playSound(const Sound& sound, float volume, glm::vec3 position, glm::vec3 velocity) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	position *= coordinateScale;
	velocity *= coordinateScale;
	return soloud.play3dClocked(time, *static_cast<SoLoud::Wav*>(sound.get()), position.x, position.y, position.z, velocity.x, velocity.y, velocity.z, volume);
}

SoundStage::SoundInstanceId SoundStage::playSoundInBackground(const Sound& sound, float volume) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	const SoLoud::handle id = soloud.playBackground(*static_cast<SoLoud::Wav*>(sound.get()), volume);
	soloud.setProtectVoice(id, true);
	return id;
}

SoundStage::SoundInstanceId SoundStage::createPausedSoundInBackground(const Sound& sound, float volume) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	const SoLoud::handle id = soloud.playBackground(*static_cast<SoLoud::Wav*>(sound.get()), volume, true);
	soloud.setProtectVoice(id, true);
	return id;
}

bool SoundStage::isSoundPaused(SoundInstanceId id) const noexcept {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	return soloud.getPause(id);
}

bool SoundStage::isSoundStopped(SoundInstanceId id) const noexcept {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	return !soloud.isValidVoiceHandle(id);
}

void SoundStage::stopSound(SoundInstanceId id) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	soloud.stop(id);
}

void SoundStage::pauseSound(SoundInstanceId id) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	soloud.setPause(id, true);
}

void SoundStage::resumeSound(SoundInstanceId id) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	soloud.setPause(id, false);
}

void SoundStage::scheduleSoundStop(SoundInstanceId id, float timePointInSound) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	soloud.scheduleStop(id, timePointInSound);
}

void SoundStage::scheduleSoundPause(SoundInstanceId id, float timePointInSound) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	soloud.schedulePause(id, timePointInSound);
}

void SoundStage::seekToSoundTime(SoundInstanceId id, float timePointInSound) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	soloud.seek(id, timePointInSound);
}

void SoundStage::setVolume(float volume) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	soloud.setGlobalVolume(volume);
}

void SoundStage::setMaxSimultaneousSounds(unsigned maxSimultaneousSounds) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	soloud.setMaxActiveVoiceCount(static_cast<unsigned>(maxSimultaneousSounds));
}

void SoundStage::EngineDeleter::operator()(void* handle) const noexcept {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(handle);
	soloud.deinit();
	delete static_cast<SoLoud::Soloud*>(handle); // NOLINT(cppcoreguidelines-owning-memory)
}

} // namespace audio
} // namespace donut
