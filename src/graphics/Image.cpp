#include <donut/InputFileStream.hpp>
#include <donut/OutputFileStream.hpp>
#include <donut/graphics/Error.hpp>
#include <donut/graphics/Image.hpp>

#include <cstddef>
#include <fmt/format.h>
#include <physfs.h>
#include <span>
#include <stb_image.h>
#include <stb_image_write.h>

namespace donut {
namespace graphics {

namespace {

constexpr stbi_io_callbacks IMAGE_FILE_INPUT_CALLBACKS{
	.read = [](void* user, char* data, int size) -> int {
		InputFileStream& file = *static_cast<InputFileStream*>(user);
		return static_cast<int>(file.read(std::as_writable_bytes(std::span{data, static_cast<std::size_t>(size)})));
	},
	.skip = [](void* user, int n) -> void {
		InputFileStream& file = *static_cast<InputFileStream*>(user);
		file.skip(static_cast<std::ptrdiff_t>(n));
	},
	.eof = [](void* user) -> int {
		InputFileStream& file = *static_cast<InputFileStream*>(user);
		return (file.eof()) ? 1 : 0;
	},
};

void imageFileOutputCallback(void* context, void* data, int size) noexcept {
	OutputFileStream& file = *static_cast<OutputFileStream*>(context);
	file.write(std::span{reinterpret_cast<const std::byte*>(data), static_cast<std::size_t>(size)});
}

} // namespace

void Image::savePng(ImageView image, const char* filepath, const ImageSavePngOptions& options) {
	OutputFileStream file = OutputFileStream::create(filepath);
	stbi_flip_vertically_on_write(options.flipVertically ? 1 : 0);
	stbi_write_png_compression_level = options.compressionLevel;
	const int width = static_cast<int>(image.getWidth());
	const int height = static_cast<int>(image.getHeight());
	const int channelCount = static_cast<int>(image.getChannelCount());
	const stbi_uc* const pixels = static_cast<const stbi_uc*>(image.getPixels());
	if (stbi_write_png_to_func(imageFileOutputCallback, &file, width, height, channelCount, pixels, 0) == 0) {
		throw Error{fmt::format("Failed to save PNG image \"{}\"!", filepath)};
	}
}

void Image::saveBmp(ImageView image, const char* filepath, const ImageSaveBmpOptions& options) {
	OutputFileStream file = OutputFileStream::create(filepath);
	stbi_flip_vertically_on_write(options.flipVertically ? 1 : 0);
	const int width = static_cast<int>(image.getWidth());
	const int height = static_cast<int>(image.getHeight());
	const int channelCount = static_cast<int>(image.getChannelCount());
	const stbi_uc* const pixels = static_cast<const stbi_uc*>(image.getPixels());
	if (stbi_write_bmp_to_func(imageFileOutputCallback, &file, width, height, channelCount, pixels) == 0) {
		throw Error{fmt::format("Failed to save BMP image \"{}\"!", filepath)};
	}
}

void Image::saveTga(ImageView image, const char* filepath, const ImageSaveTgaOptions& options) {
	OutputFileStream file = OutputFileStream::create(filepath);
	stbi_flip_vertically_on_write(options.flipVertically ? 1 : 0);
	stbi_write_tga_with_rle = (options.useRleCompression) ? 1 : 0;
	const int width = static_cast<int>(image.getWidth());
	const int height = static_cast<int>(image.getHeight());
	const int channelCount = static_cast<int>(image.getChannelCount());
	const stbi_uc* const pixels = static_cast<const stbi_uc*>(image.getPixels());
	if (stbi_write_tga_to_func(imageFileOutputCallback, &file, width, height, channelCount, pixels) == 0) {
		throw Error{fmt::format("Failed to save TGA image \"{}\"!", filepath)};
	}
}

void Image::saveJpg(ImageView image, const char* filepath, const ImageSaveJpgOptions& options) {
	OutputFileStream file = OutputFileStream::create(filepath);
	stbi_flip_vertically_on_write(options.flipVertically ? 1 : 0);
	const int width = static_cast<int>(image.getWidth());
	const int height = static_cast<int>(image.getHeight());
	const int channelCount = static_cast<int>(image.getChannelCount());
	const stbi_uc* const pixels = static_cast<const stbi_uc*>(image.getPixels());
	if (stbi_write_jpg_to_func(imageFileOutputCallback, &file, width, height, channelCount, pixels, options.quality) == 0) {
		throw Error{fmt::format("Failed to save JPG image \"{}\"!", filepath)};
	}
}

void Image::saveHdr(ImageView image, const char* filepath, const ImageSaveHdrOptions& options) {
	OutputFileStream file = OutputFileStream::create(filepath);
	stbi_flip_vertically_on_write(options.flipVertically ? 1 : 0);
	const int width = static_cast<int>(image.getWidth());
	const int height = static_cast<int>(image.getHeight());
	const int channelCount = static_cast<int>(image.getChannelCount());
	const float* const pixels = static_cast<const float*>(image.getPixels());
	if (stbi_write_hdr_to_func(imageFileOutputCallback, &file, width, height, channelCount, pixels) == 0) {
		throw Error{fmt::format("Failed to save HDR image \"{}\"!", filepath)};
	}
}

Image::Image(const char* filepath, const ImageOptions& options) {
	stbi_set_flip_vertically_on_load_thread(options.flipVertically ? 1 : 0);
	int widthValue = 0;
	int heightValue = 0;
	int channelCountValue = 0;
	InputFileStream file = InputFileStream::open(filepath);
	if (options.highDynamicRange) {
		pixels.reset(stbi_loadf_from_callbacks(&IMAGE_FILE_INPUT_CALLBACKS, &file, &widthValue, &heightValue, &channelCountValue, options.desiredChannelCount));
		if (!pixels) {
			throw Error{fmt::format("Failed to load HDR image \"{}\": {}", filepath, stbi_failure_reason())};
		}
	} else {
		pixels.reset(stbi_load_from_callbacks(&IMAGE_FILE_INPUT_CALLBACKS, &file, &widthValue, &heightValue, &channelCountValue, options.desiredChannelCount));
		if (!pixels) {
			throw Error{fmt::format("Failed to load image \"{}\": {}", filepath, stbi_failure_reason())};
		}
	}
	width = static_cast<std::size_t>(widthValue);
	height = static_cast<std::size_t>(heightValue);
	channelCount = static_cast<std::size_t>(channelCountValue);
}

void Image::PixelsDeleter::operator()(void* handle) const noexcept {
	stbi_image_free(handle);
}

} // namespace graphics
} // namespace donut
