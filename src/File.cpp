#include <donut/File.hpp>

#include <cstddef>      // std::size_t, std::ptrdiff_t, std::byte
#include <fmt/format.h> // fmt::format
#include <physfs.h>     // PHYSFS_...
#include <span>         // std::span, std::as_writable_bytes
#include <string>       // std::string
#include <vector>       // std::vector

namespace donut {

void File::close() {
	if (file) {
		if (PHYSFS_close(static_cast<PHYSFS_File*>(file.get())) == 0) {
			throw Error{fmt::format("Failed to close file: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
		}
		file.release();
	}
}

bool File::eof() const noexcept {
	return PHYSFS_eof(static_cast<PHYSFS_File*>(file.get())) != 0;
}

size_t File::size() const noexcept {
	const PHYSFS_sint64 length = PHYSFS_fileLength(static_cast<PHYSFS_File*>(file.get()));
	return (length < 0) ? NPOS : static_cast<size_t>(length);
}

size_t File::tellg() const noexcept {
	const PHYSFS_sint64 position = PHYSFS_tell(static_cast<PHYSFS_File*>(file.get()));
	return (position < 0) ? NPOS : static_cast<size_t>(position);
}

size_t File::tellp() const noexcept {
	const PHYSFS_sint64 position = PHYSFS_tell(static_cast<PHYSFS_File*>(file.get()));
	return (position < 0) ? NPOS : static_cast<size_t>(position);
}

void File::seekg(size_t position) {
	if (PHYSFS_seek(static_cast<PHYSFS_File*>(file.get()), static_cast<PHYSFS_uint64>(position)) == 0) {
		throw Error{fmt::format("Failed to seek in resource file:\n{}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
}

void File::seekp(size_t position) {
	if (PHYSFS_seek(static_cast<PHYSFS_File*>(file.get()), static_cast<PHYSFS_uint64>(position)) == 0) {
		throw Error{fmt::format("Failed to seek in resource file:\n{}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
}

void File::skipg(ptrdiff_t offset) {
	if (const PHYSFS_sint64 position = PHYSFS_tell(static_cast<PHYSFS_File*>(file.get())); position > 0) {
		if (PHYSFS_seek(static_cast<PHYSFS_File*>(file.get()), static_cast<PHYSFS_uint64>(position + offset)) == 0) {
			throw Error{fmt::format("Failed to seek in resource file:\n{}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
		}
	} else {
		throw Error{fmt::format("Failed to get position in resource file:\n{}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
}

void File::skipp(ptrdiff_t offset) {
	if (const PHYSFS_sint64 position = PHYSFS_tell(static_cast<PHYSFS_File*>(file.get())); position > 0) {
		if (PHYSFS_seek(static_cast<PHYSFS_File*>(file.get()), static_cast<PHYSFS_uint64>(position + offset)) == 0) {
			throw Error{fmt::format("Failed to seek in resource file:\n{}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
		}
	} else {
		throw Error{fmt::format("Failed to get position in resource file:\n{}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
}

std::vector<std::byte> File::readAll() && {
	std::vector<std::byte> result{};
	seekg(0);
	result.resize(size());
	if (read(result) != result.size()) {
		throw Error{fmt::format("Failed to read file contents: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
	return result;
}

std::string File::readAllIntoString() && {
	std::string result{};
	seekg(0);
	result.resize(size());
	if (read(std::as_writable_bytes(std::span{result})) != result.size()) {
		throw Error{fmt::format("Failed to read file contents: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
	return result;
}

std::size_t File::read(std::span<std::byte> data) {
	const PHYSFS_sint64 bytesRead = PHYSFS_readBytes(static_cast<PHYSFS_File*>(file.get()), data.data(), static_cast<PHYSFS_uint64>(data.size()));
	if (bytesRead < 0) {
		throw Error{fmt::format("Failed to read from resource file:\n{}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
	return static_cast<size_t>(bytesRead);
}

size_t File::write(std::span<const std::byte> data) {
	const PHYSFS_sint64 bytesWritten = PHYSFS_writeBytes(static_cast<PHYSFS_File*>(file.get()), data.data(), static_cast<PHYSFS_uint64>(data.size()));
	if (bytesWritten < 0) {
		throw Error{fmt::format("Failed to write to resource file:\n{}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
	return static_cast<size_t>(bytesWritten);
}

void File::flush() {
	if (PHYSFS_flush(static_cast<PHYSFS_File*>(file.get())) == 0) {
		throw Error{fmt::format("Failed to flush resource file:\n{}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
}

void File::FileDeleter::operator()(void* handle) const noexcept {
	if (handle) {
		PHYSFS_close(static_cast<PHYSFS_File*>(handle));
	}
}

} // namespace donut
