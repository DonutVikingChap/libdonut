#include <donut/File.hpp>
#include <donut/Filesystem.hpp>
#include <donut/audio/Error.hpp>
#include <donut/audio/Sound.hpp>

#include <cstddef>       // std::size_t
#include <fmt/format.h>  // fmt::format
#include <soloud.h>      // SoLoud::...
#include <soloud_file.h> // SoLoud::File
#include <soloud_wav.h>  // SoLoud::Wav
#include <span>          // std::span, std::as_writable_bytes

namespace donut::audio {

namespace {

class SoundFile final : public SoLoud::File {
public:
	explicit SoundFile(const Filesystem& filesystem, const char* filepath)
		: file(filesystem.openFile(filepath)) {}

	~SoundFile() override = default;

	SoundFile(const SoundFile&) = delete;
	SoundFile(SoundFile&&) = delete;
	SoundFile& operator=(const SoundFile&) = delete;
	SoundFile& operator=(SoundFile&&) = delete;

	int eof() override {
		return (file.eof()) ? 1 : 0;
	}

	unsigned read(unsigned char* aDst, unsigned aBytes) override {
		return static_cast<unsigned>(file.read(std::as_writable_bytes(std::span{aDst, static_cast<std::size_t>(aBytes)})));
	}

	unsigned length() override {
		return static_cast<unsigned>(file.size());
	}

	void seek(int aOffset) override {
		file.seekg(static_cast<std::size_t>(aOffset));
	}

	unsigned pos() override {
		return static_cast<unsigned>(file.tellg());
	}

private:
	donut::File file;
};

[[nodiscard]] constexpr unsigned getSoLoudAttenuationModel(SoundAttenuationModel attenuationModel) noexcept {
	switch (attenuationModel) {
		case SoundAttenuationModel::NO_ATTENUATION: return SoLoud::AudioSource::NO_ATTENUATION;
		case SoundAttenuationModel::INVERSE_DISTANCE: return SoLoud::AudioSource::INVERSE_DISTANCE;
		case SoundAttenuationModel::LINEAR_DISTANCE: return SoLoud::AudioSource::LINEAR_DISTANCE;
		case SoundAttenuationModel::EXPONENTIAL_DISTANCE: return SoLoud::AudioSource::EXPONENTIAL_DISTANCE;
	}
	return 0u;
}

} // namespace

Sound::Sound(const Filesystem& filesystem, const char* filepath, const SoundOptions& options)
	: buffer(new SoLoud::Wav{}) {
	SoLoud::Wav& wav = *static_cast<SoLoud::Wav*>(buffer.get());
	SoundFile file{filesystem, filepath};
	if (const SoLoud::result errorCode = wav.loadFile(&file); errorCode != SoLoud::SO_NO_ERROR) {
		throw Error{fmt::format("Failed to load sound file \"{}\"", filepath), errorCode};
	}
	wav.setVolume(options.volume);
	wav.setInaudibleBehavior(false, true);
	wav.set3dMinMaxDistance(options.minDistance, options.maxDistance);
	wav.set3dAttenuation(getSoLoudAttenuationModel(options.attenuationModel), options.rolloffFactor);
	wav.set3dDopplerFactor(options.dopplerFactor);
	wav.set3dDistanceDelay(options.useDistanceDelay);
	wav.set3dListenerRelative(options.listenerRelative);
	wav.setLooping(options.looping);
	wav.setSingleInstance(options.singleInstance);
}

void Sound::SourceDeleter::operator()(void* handle) const noexcept {
	delete static_cast<SoLoud::Wav*>(handle); // NOLINT(cppcoreguidelines-owning-memory)
}

} // namespace donut::audio
