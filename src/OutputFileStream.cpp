#include <donut/OutputFileStream.hpp>

#include <cstddef>  // std::size_t, std::byte
#include <format>   // std::format
#include <physfs.h> // PHYSFS_...
#include <span>     // std::span

namespace donut {

OutputFileStream OutputFileStream::create(const char* filepath) {
	OutputFileStream result{PHYSFS_openWrite(filepath)};
	if (!result) {
		throw Error{std::format("Failed to create file \"{}\" for writing: {}", filepath, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
	return result;
}

OutputFileStream OutputFileStream::append(const char* filepath) {
	OutputFileStream result{PHYSFS_openAppend(filepath)};
	if (!result) {
		throw Error{std::format("Failed to open file \"{}\" for appending: {}", filepath, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
	return result;
}

std::size_t OutputFileStream::write(std::span<const std::byte> data) noexcept {
	const PHYSFS_sint64 bytesWritten = PHYSFS_writeBytes(static_cast<PHYSFS_File*>(file.get()), data.data(), static_cast<PHYSFS_uint64>(data.size()));
	return (bytesWritten < 0) ? std::size_t{0} : static_cast<std::size_t>(bytesWritten);
}

} // namespace donut
