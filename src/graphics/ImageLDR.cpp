#include <donut/OutputFileStream.hpp>
#include <donut/graphics/Error.hpp>
#include <donut/graphics/ImageLDR.hpp>

#include <cstddef>           // std::size_t, std::byte
#include <cstring>           // std::memcpy
#include <fmt/format.h>      // fmt::format
#include <new>               // std::bad_alloc
#include <span>              // std::span
#include <stb_image_write.h> // stbi_..., stbi_write_...
#include <stdlib.h>          // malloc // NOLINT(modernize-deprecated-headers)

namespace donut {
namespace graphics {

namespace {

void imageFileOutputCallback(void* context, void* data, int size) noexcept {
	OutputFileStream& file = *static_cast<OutputFileStream*>(context);
	file.write(std::span{reinterpret_cast<const std::byte*>(data), static_cast<std::size_t>(size)});
}

} // namespace

void ImageLDR::savePNG(const ImageLDRView& image, const char* filepath, const ImageLDRSavePNGOptions& options) {
	OutputFileStream file = OutputFileStream::create(filepath);
	stbi_flip_vertically_on_write(options.flipVertically ? 1 : 0);
	stbi_write_png_compression_level = options.compressionLevel;
	const int width = static_cast<int>(image.getWidth());
	const int height = static_cast<int>(image.getHeight());
	const int channelCount = static_cast<int>(image.getChannelCount());
	const std::byte* const pixels = image.getPixels();
	if (stbi_write_png_to_func(imageFileOutputCallback, &file, width, height, channelCount, pixels, 0) == 0) {
		throw Error{fmt::format("Failed to save PNG image \"{}\"!", filepath)};
	}
}

void ImageLDR::saveBMP(const ImageLDRView& image, const char* filepath, const ImageLDRSaveBMPOptions& options) {
	OutputFileStream file = OutputFileStream::create(filepath);
	stbi_flip_vertically_on_write(options.flipVertically ? 1 : 0);
	const int width = static_cast<int>(image.getWidth());
	const int height = static_cast<int>(image.getHeight());
	const int channelCount = static_cast<int>(image.getChannelCount());
	const std::byte* const pixels = image.getPixels();
	if (stbi_write_bmp_to_func(imageFileOutputCallback, &file, width, height, channelCount, pixels) == 0) {
		throw Error{fmt::format("Failed to save BMP image \"{}\"!", filepath)};
	}
}

void ImageLDR::saveTGA(const ImageLDRView& image, const char* filepath, const ImageLDRSaveTGAOptions& options) {
	OutputFileStream file = OutputFileStream::create(filepath);
	stbi_flip_vertically_on_write(options.flipVertically ? 1 : 0);
	stbi_write_tga_with_rle = (options.useRleCompression) ? 1 : 0;
	const int width = static_cast<int>(image.getWidth());
	const int height = static_cast<int>(image.getHeight());
	const int channelCount = static_cast<int>(image.getChannelCount());
	const std::byte* const pixels = image.getPixels();
	if (stbi_write_tga_to_func(imageFileOutputCallback, &file, width, height, channelCount, pixels) == 0) {
		throw Error{fmt::format("Failed to save TGA image \"{}\"!", filepath)};
	}
}

void ImageLDR::saveJPG(const ImageLDRView& image, const char* filepath, const ImageLDRSaveJPGOptions& options) {
	OutputFileStream file = OutputFileStream::create(filepath);
	stbi_flip_vertically_on_write(options.flipVertically ? 1 : 0);
	const int width = static_cast<int>(image.getWidth());
	const int height = static_cast<int>(image.getHeight());
	const int channelCount = static_cast<int>(image.getChannelCount());
	const std::byte* const pixels = image.getPixels();
	if (stbi_write_jpg_to_func(imageFileOutputCallback, &file, width, height, channelCount, pixels, options.quality) == 0) {
		throw Error{fmt::format("Failed to save JPG image \"{}\"!", filepath)};
	}
}

ImageLDR::ImageLDR(const std::byte* pixels, std::size_t width, std::size_t height, std::size_t channelCount)
	: Image(Image::Pixels{malloc(width * height * channelCount * sizeof(std::byte))}, // NOLINT(cppcoreguidelines-no-malloc)
		  width, height, channelCount) {
	if (!getPixels()) {
		throw std::bad_alloc{};
	}
	std::memcpy(getPixels(), pixels, width * height * channelCount * sizeof(std::byte));
}

} // namespace graphics
} // namespace donut
