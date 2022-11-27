#include <donut/InputFileStream.hpp>

#include <cstddef>
#include <fmt/format.h>
#include <physfs.h>
#include <span>
#include <string>
#include <vector>

namespace donut {

InputFileStream InputFileStream::open(const char* filepath) {
	InputFileStream result{PHYSFS_openRead(filepath)};
	if (!result) {
		throw Error{fmt::format("Failed to open file \"{}\" for reading: {}", filepath, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
	return result;
}

std::vector<std::byte> InputFileStream::readAll() && {
	std::vector<std::byte> result{};
	seek(0);
	result.resize(size());
	if (read(result) != result.size()) {
		throw Error{fmt::format("Failed to read file contents: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
	return result;
}

std::string InputFileStream::readAllIntoString() && {
	std::string result{};
	seek(0);
	result.resize(size());
	if (read(std::as_writable_bytes(std::span{result})) != result.size()) {
		throw Error{fmt::format("Failed to read file contents: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
	return result;
}

std::size_t InputFileStream::size() const noexcept {
	return static_cast<std::size_t>(PHYSFS_fileLength(static_cast<PHYSFS_File*>(file.get())));
}

std::size_t InputFileStream::tell() const noexcept {
	return static_cast<std::size_t>(PHYSFS_tell(static_cast<PHYSFS_File*>(file.get())));
}

std::size_t InputFileStream::read(std::span<std::byte> data) noexcept {
	const PHYSFS_sint64 bytesRead = PHYSFS_readBytes(static_cast<PHYSFS_File*>(file.get()), data.data(), static_cast<PHYSFS_uint64>(data.size()));
	return (bytesRead < 0) ? std::size_t{0} : static_cast<std::size_t>(bytesRead);
}

void InputFileStream::seek(std::size_t position) const noexcept {
	PHYSFS_seek(static_cast<PHYSFS_File*>(file.get()), static_cast<PHYSFS_uint64>(position));
}

void InputFileStream::skip(std::ptrdiff_t n) noexcept {
	if (const PHYSFS_sint64 position = PHYSFS_tell(static_cast<PHYSFS_File*>(file.get())); position > 0) {
		PHYSFS_seek(static_cast<PHYSFS_File*>(file.get()), static_cast<PHYSFS_uint64>(position + n));
	}
}

bool InputFileStream::eof() const noexcept {
	return PHYSFS_eof(static_cast<PHYSFS_File*>(file.get())) != 0;
}

} // namespace donut
