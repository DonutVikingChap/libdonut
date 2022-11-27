#ifndef DONUT_INPUT_FILE_STREAM_HPP
#define DONUT_INPUT_FILE_STREAM_HPP

#include <donut/File.hpp>

#include <cstddef>
#include <span>
#include <string>
#include <vector>

namespace donut {

class InputFileStream : public File {
public:
	[[nodiscard]] static InputFileStream open(const char* filepath);

	constexpr InputFileStream() noexcept = default;

	[[nodiscard]] std::vector<std::byte> readAll() &&;
	[[nodiscard]] std::string readAllIntoString() &&;

	[[nodiscard]] std::size_t size() const noexcept;
	[[nodiscard]] std::size_t tell() const noexcept;

	[[nodiscard]] std::size_t read(std::span<std::byte> data) noexcept;

	void seek(std::size_t position) const noexcept;
	void skip(std::ptrdiff_t n) noexcept;

	[[nodiscard]] bool eof() const noexcept;

private:
	constexpr explicit InputFileStream(void* handle) noexcept
		: File(handle) {}
};

} // namespace donut

#endif
