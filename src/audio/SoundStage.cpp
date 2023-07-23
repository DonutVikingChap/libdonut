#include <donut/Time.hpp>
#include <donut/audio/Error.hpp>
#include <donut/audio/Listener.hpp>
#include <donut/audio/Sound.hpp>
#include <donut/audio/SoundStage.hpp>
#include <donut/math.hpp>

#include <soloud.h>     // SoLoud::...
#include <soloud_wav.h> // SoLoud::Wav

namespace donut::audio {

SoundStage::SoundStage(const SoundStageOptions& options)
	: engine(new SoLoud::Soloud{}) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	if (const SoLoud::result errorCode = soloud.init(); errorCode != SoLoud::SO_NO_ERROR) {
		throw Error{"Failed to initialize sound stage", errorCode};
	}
	setVolume(options.volume);
	setSpeedOfSound(options.speedOfSound);
	setMaxSimultaneousSounds(options.maxSimultaneousSounds);
}

void SoundStage::update(Time<float> deltaTime, const Listener& listener) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());

	time += deltaTime;

	soloud.set3dListenerParameters(listener.position.x, listener.position.y, listener.position.z, listener.aimDirection.x, listener.aimDirection.y, listener.aimDirection.z,
		listener.up.x, listener.up.y, listener.up.z, listener.velocity.x, listener.velocity.y, listener.velocity.z);

	soloud.update3dAudio();
}

SoundStage::SoundInstanceId SoundStage::playSound(const Sound& sound, float volume, vec3 position, vec3 velocity) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
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

void SoundStage::scheduleSoundStop(SoundInstanceId id, Time<float> timePointInSound) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	soloud.scheduleStop(id, timePointInSound);
}

void SoundStage::scheduleSoundPause(SoundInstanceId id, Time<float> timePointInSound) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	soloud.schedulePause(id, timePointInSound);
}

void SoundStage::seekToSoundTime(SoundInstanceId id, Time<float> timePointInSound) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	soloud.seek(id, timePointInSound);
}

void SoundStage::setSoundPosition(SoundInstanceId id, vec3 newPosition) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	soloud.set3dSourcePosition(id, newPosition.x, newPosition.y, newPosition.z);
}

void SoundStage::setSoundVelocity(SoundInstanceId id, vec3 newVelocity) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	soloud.set3dSourceVelocity(id, newVelocity.x, newVelocity.y, newVelocity.z);
}

void SoundStage::setSoundPositionAndVelocity(SoundInstanceId id, vec3 newPosition, vec3 newVelocity) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	soloud.set3dSourceParameters(id, newPosition.x, newPosition.y, newPosition.z, newVelocity.x, newVelocity.y, newVelocity.z);
}

void SoundStage::setSoundVolume(SoundInstanceId id, float volume) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	soloud.setVolume(id, volume);
}

void SoundStage::fadeSoundVolume(SoundInstanceId id, float targetVolume, float fadeDuration) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	soloud.fadeVolume(id, targetVolume, fadeDuration);
}

void SoundStage::setSoundPlaybackSpeed(SoundInstanceId id, float playbackSpeed) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	soloud.setRelativePlaySpeed(id, playbackSpeed);
}

void SoundStage::fadeSoundPlaybackSpeed(SoundInstanceId id, float targetPlaybackSpeed, float fadeDuration) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	soloud.fadeRelativePlaySpeed(id, targetPlaybackSpeed, fadeDuration);
}

void SoundStage::setVolume(float volume) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	soloud.setGlobalVolume(volume);
}

void SoundStage::fadeVolume(float targetVolume, float fadeDuration) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	soloud.fadeGlobalVolume(targetVolume, fadeDuration);
}

void SoundStage::setSpeedOfSound(float speedOfSound) {
	SoLoud::Soloud& soloud = *static_cast<SoLoud::Soloud*>(engine.get());
	soloud.set3dSoundSpeed(speedOfSound);
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

} // namespace donut::audio
