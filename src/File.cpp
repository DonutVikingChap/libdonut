#include <donut/File.hpp>

#include <fmt/format.h> // fmt::format
#include <physfs.h>     // PHYSFS_...

namespace donut {

void File::createDirectory(const char* filepath) {
	if (PHYSFS_mkdir(filepath) == 0) {
		throw Error{fmt::format("Failed to create directory \"{}\": {}", filepath, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
}

void File::deleteFile(const char* filepath) {
	if (PHYSFS_delete(filepath) == 0) {
		throw Error{fmt::format("Failed to delete file \"{}\": {}", filepath, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
}

bool File::exists(const char* filepath) {
	return PHYSFS_exists(filepath) != 0;
}

void File::close() {
	if (file) {
		if (PHYSFS_close(static_cast<PHYSFS_File*>(file.get())) == 0) {
			throw Error{fmt::format("Failed to close file: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
		}
		file.release();
	}
}

void File::FileDeleter::operator()(void* handle) const noexcept {
	if (handle) {
		PHYSFS_close(static_cast<PHYSFS_File*>(handle));
	}
}

} // namespace donut
