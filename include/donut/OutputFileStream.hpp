#ifndef DONUT_OUTPUT_FILE_STREAM_HPP
#define DONUT_OUTPUT_FILE_STREAM_HPP

#include <donut/File.hpp>

#include <cstddef>
#include <span>

namespace donut {

class OutputFileStream : public File {
public:
	[[nodiscard]] static OutputFileStream create(const char* filepath);
	[[nodiscard]] static OutputFileStream append(const char* filepath);

	constexpr OutputFileStream() noexcept = default;

	std::size_t write(std::span<const std::byte> data) noexcept;

private:
	constexpr explicit OutputFileStream(void* handle) noexcept
		: File(handle) {}
};

} // namespace donut

#endif
