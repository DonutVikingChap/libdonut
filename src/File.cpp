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

File::Metadata File::getFileMetadata(const char* filepath) {
	PHYSFS_Stat metadata{};
	if (PHYSFS_stat(filepath, &metadata) == 0) {
		throw Error{fmt::format("Failed to get file metadata: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
	Kind kind{};
	switch (metadata.filetype) {
		case PHYSFS_FILETYPE_REGULAR: kind = Kind::REGULAR; break;
		case PHYSFS_FILETYPE_DIRECTORY: kind = Kind::DIRECTORY; break;
		case PHYSFS_FILETYPE_SYMLINK: kind = Kind::SYMLINK; break;
		case PHYSFS_FILETYPE_OTHER: kind = Kind::OTHER; break;
	}
	return {
		.size = (metadata.filesize < 0) ? NPOS : static_cast<std::size_t>(metadata.filesize),
		.creationTime = metadata.createtime,
		.lastAccessTime = metadata.accesstime,
		.lastModificationTime = metadata.modtime,
		.kind = kind,
		.readOnly = metadata.readonly != 0,
	};
}

std::vector<std::string> File::getFilenamesInDirectory(const char* filepath) {
	std::vector<std::string> result{};
	if (PHYSFS_enumerate(
			filepath,
			[](void* data, const char*, const char* fname) -> PHYSFS_EnumerateCallbackResult {
				static_cast<std::vector<std::string>*>(data)->emplace_back(fname);
				return PHYSFS_ENUM_OK;
			},
			&result) == 0) {
		throw Error{fmt::format("Failed to enumerate directory: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
	return result;
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
