#include <donut/File.hpp>
#include <donut/Filesystem.hpp>
#include <donut/graphics/Error.hpp>
#include <donut/graphics/Image.hpp>

#include <cstddef>           // std::size_t, std::ptrdiff_t, std::byte
#include <cstring>           // std::memcpy
#include <format>            // std::format
#include <new>               // std::bad_alloc
#include <span>              // std::span, std::as_writable_bytes
#include <stb_image.h>       // stbi_...
#include <stb_image_write.h> // stbi_..., stbi_write_...
#include <stdlib.h>          // malloc // NOLINT(modernize-deprecated-headers)

namespace donut::graphics {

namespace {

constexpr stbi_io_callbacks IMAGE_FILE_INPUT_CALLBACKS{
	.read = [](void* user, char* data, int size) -> int {
		File& file = *static_cast<File*>(user);
		return static_cast<int>(file.read(std::as_writable_bytes(std::span{data, static_cast<std::size_t>(size)})));
	},
	.skip = [](void* user, int n) -> void {
		File& file = *static_cast<File*>(user);
		file.skipg(static_cast<std::ptrdiff_t>(n));
	},
	.eof = [](void* user) -> int {
		File& file = *static_cast<File*>(user);
		return (file.eof()) ? 1 : 0;
	},
};

void imageFileOutputCallback(void* context, void* data, int size) noexcept {
	File& file = *static_cast<File*>(context);
	file.write(std::span{reinterpret_cast<const std::byte*>(data), static_cast<std::size_t>(size)});
}

} // namespace

void Image::savePNG(const ImageView& image, Filesystem& filesystem, const char* filepath, const ImageSavePNGOptions& options) {
	if (image.getPixelComponentType() != PixelComponentType::U8) {
		throw Error{std::format("Cannot save image to \"{}\" as PNG since the image is not stored in 8-bit unsigned integer format.", filepath)};
	}
	File file = filesystem.createFile(filepath);
	stbi_flip_vertically_on_write(options.flipVertically ? 0 : 1); // NOTE: Inverted on purpose.
	stbi_write_png_compression_level = options.compressionLevel;
	const int width = static_cast<int>(image.getWidth());
	const int height = static_cast<int>(image.getHeight());
	const int channelCount = static_cast<int>(image.getChannelCount());
	const void* const pixels = image.getPixels();
	if (stbi_write_png_to_func(imageFileOutputCallback, &file, width, height, channelCount, pixels, 0) == 0) {
		throw Error{std::format("Failed to save PNG image \"{}\"!", filepath)};
	}
}

void Image::saveBMP(const ImageView& image, Filesystem& filesystem, const char* filepath, const ImageSaveBMPOptions& options) {
	if (image.getPixelComponentType() != PixelComponentType::U8) {
		throw Error{std::format("Cannot save image to \"{}\" as BMP since the image is not stored in 8-bit unsigned integer format.", filepath)};
	}
	File file = filesystem.createFile(filepath);
	stbi_flip_vertically_on_write(options.flipVertically ? 0 : 1); // NOTE: Inverted on purpose.
	const int width = static_cast<int>(image.getWidth());
	const int height = static_cast<int>(image.getHeight());
	const int channelCount = static_cast<int>(image.getChannelCount());
	const void* const pixels = image.getPixels();
	if (stbi_write_bmp_to_func(imageFileOutputCallback, &file, width, height, channelCount, pixels) == 0) {
		throw Error{std::format("Failed to save BMP image \"{}\"!", filepath)};
	}
}

void Image::saveTGA(const ImageView& image, Filesystem& filesystem, const char* filepath, const ImageSaveTGAOptions& options) {
	if (image.getPixelComponentType() != PixelComponentType::U8) {
		throw Error{std::format("Cannot save image to \"{}\" as TGA since the image is not stored in 8-bit unsigned integer format.", filepath)};
	}
	File file = filesystem.createFile(filepath);
	stbi_flip_vertically_on_write(options.flipVertically ? 0 : 1); // NOTE: Inverted on purpose.
	stbi_write_tga_with_rle = (options.useRleCompression) ? 1 : 0;
	const int width = static_cast<int>(image.getWidth());
	const int height = static_cast<int>(image.getHeight());
	const int channelCount = static_cast<int>(image.getChannelCount());
	const void* const pixels = image.getPixels();
	if (stbi_write_tga_to_func(imageFileOutputCallback, &file, width, height, channelCount, pixels) == 0) {
		throw Error{std::format("Failed to save TGA image \"{}\"!", filepath)};
	}
}

void Image::saveJPG(const ImageView& image, Filesystem& filesystem, const char* filepath, const ImageSaveJPGOptions& options) {
	if (image.getPixelComponentType() != PixelComponentType::U8) {
		throw Error{std::format("Cannot save image to \"{}\" as JPG since the image is not stored in 8-bit unsigned integer format.", filepath)};
	}
	File file = filesystem.createFile(filepath);
	stbi_flip_vertically_on_write(options.flipVertically ? 0 : 1); // NOTE: Inverted on purpose.
	const int width = static_cast<int>(image.getWidth());
	const int height = static_cast<int>(image.getHeight());
	const int channelCount = static_cast<int>(image.getChannelCount());
	const void* const pixels = image.getPixels();
	if (stbi_write_jpg_to_func(imageFileOutputCallback, &file, width, height, channelCount, pixels, options.quality) == 0) {
		throw Error{std::format("Failed to save JPG image \"{}\"!", filepath)};
	}
}

void Image::saveHDR(const ImageView& image, Filesystem& filesystem, const char* filepath, const ImageSaveHDROptions& options) {
	if (image.getPixelComponentType() != PixelComponentType::F32) {
		throw Error{std::format("Cannot save image to \"{}\" as HDR since the image is not stored in 32-bit floating-point format.", filepath)};
	}
	File file = filesystem.createFile(filepath);
	stbi_flip_vertically_on_write(options.flipVertically ? 0 : 1); // NOTE: Inverted on purpose.
	const int width = static_cast<int>(image.getWidth());
	const int height = static_cast<int>(image.getHeight());
	const int channelCount = static_cast<int>(image.getChannelCount());
	const float* const pixels = static_cast<const float*>(image.getPixels());
	if (stbi_write_hdr_to_func(imageFileOutputCallback, &file, width, height, channelCount, pixels) == 0) {
		throw Error{std::format("Failed to save HDR image \"{}\"!", filepath)};
	}
}

void Image::save(const ImageView& image, Filesystem& filesystem, const char* filepath, const ImageSaveOptions& options) {
	if (image.getPixelComponentType() == PixelComponentType::U8) {
		savePNG(image, filesystem, filepath, ImageSavePNGOptions{.flipVertically = options.flipVertically});
	} else if (image.getPixelComponentType() == PixelComponentType::F32) {
		saveHDR(image, filesystem, filepath, ImageSaveHDROptions{.flipVertically = options.flipVertically});
	} else {
		throw Error{std::format("Cannot save image to \"{}\" since the image is not stored in a suitable format.", filepath)};
	}
}

Image::Image(std::size_t width, std::size_t height, PixelFormat pixelFormat, PixelComponentType pixelComponentType, const void* pixels)
	: Image(ImageView{width, height, pixelFormat, pixelComponentType, pixels}) {}

Image::Image(const ImageView& image)
	: pixels(malloc(image.getSizeInBytes())) // NOLINT(cppcoreguidelines-no-malloc)
	, width(image.getWidth())
	, height(image.getHeight())
	, pixelFormat(image.getPixelFormat())
	, pixelComponentType(image.getPixelComponentType()) {
	if (!pixels) {
		throw std::bad_alloc{};
	}
	std::memcpy(pixels.get(), image.getPixels(), image.getSizeInBytes());
}

Image::Image(const Filesystem& filesystem, const char* filepath, const ImageOptions& options) {
	int desiredChannelCount = 0;
	if (options.desiredFormat) {
		switch (*options.desiredFormat) {
			case PixelFormat::R: desiredChannelCount = 1; break;
			case PixelFormat::RG: desiredChannelCount = 2; break;
			case PixelFormat::RGB: desiredChannelCount = 3; break;
			case PixelFormat::RGBA: desiredChannelCount = 4; break;
		}
	}
	stbi_set_flip_vertically_on_load_thread(options.flipVertically ? 0 : 1); // NOTE: Inverted on purpose.
	int imageWidth = 0;
	int imageHeight = 0;
	int channelsInFile = 0;
	File file = filesystem.openFile(filepath);
	if (options.highDynamicRange) {
		pixels.reset(stbi_loadf_from_callbacks(&IMAGE_FILE_INPUT_CALLBACKS, &file, &imageWidth, &imageHeight, &channelsInFile, desiredChannelCount));
		if (!pixels) {
			throw Error{std::format("Failed to load HDR image \"{}\": {}", filepath, stbi_failure_reason())};
		}
		pixelComponentType = PixelComponentType::F32;
	} else {
		pixels.reset(stbi_load_from_callbacks(&IMAGE_FILE_INPUT_CALLBACKS, &file, &imageWidth, &imageHeight, &channelsInFile, desiredChannelCount));
		if (!pixels) {
			throw Error{std::format("Failed to load image \"{}\": {}", filepath, stbi_failure_reason())};
		}
		pixelComponentType = PixelComponentType::U8;
	}
	width = static_cast<std::size_t>(imageWidth);
	height = static_cast<std::size_t>(imageHeight);
	if (options.desiredFormat) {
		pixelFormat = *options.desiredFormat;
	} else {
		switch (channelsInFile) {
			case 1: pixelFormat = PixelFormat::R; break;
			case 2: pixelFormat = PixelFormat::RG; break;
			case 3: pixelFormat = PixelFormat::RGB; break;
			case 4: pixelFormat = PixelFormat::RGBA; break;
			default: throw Error{std::format("Invalid pixel format in image \"{}\".", filepath)};
		}
	}
}

void Image::PixelsDeleter::operator()(void* handle) const noexcept {
	stbi_image_free(handle);
}

} // namespace donut::graphics
