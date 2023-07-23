#include <donut/File.hpp>
#include <donut/Filesystem.hpp>

#include <cassert>  // assert
#include <cstddef>  // std::size_t
#include <cstring>  // std::strlen
#include <format>   // std::format
#include <physfs.h> // PHYSFS_...
#include <span>     // std::span
#include <string>   // std::string
#include <utility>  // std::move
#include <vector>   // std::vector

namespace donut {

Filesystem::Filesystem(const char* programFilepath, const FilesystemOptions& options) {
	if (PHYSFS_init(programFilepath) == 0) {
		throw File::Error{std::format("Failed to initialize filesystem:\n{}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
	if (options.organizationName && options.applicationName) {
		if (std::string standardOutputDirectory = getStandardOutputDirectory(options.organizationName, options.applicationName); !standardOutputDirectory.empty()) {
			setOutputDirectory(std::move(standardOutputDirectory));
			if (options.mountOutputDirectory) {
				mountArchive(outputDirectory.c_str());
			}
		}
	}
	if (options.dataDirectory) {
		mountArchive(options.dataDirectory, options.mountPriorityOfDataDirectoryRelativeToOutputDirectory);
	}
	if (options.archiveSearchPath) {
		if (options.mountPriorityOfArchiveSearchRelativeToOutputDirectory == FilesystemMountPriority::LOWER &&
			options.mountPriorityOfArchiveSearchRelativeToDataDirectory == FilesystemMountPriority::LOWER) {
			mountArchives(options.archiveSearchPath, options.archiveSearchFileExtension, FilesystemMountPriority::LOWER);
		} else if (options.mountPriorityOfArchiveSearchRelativeToOutputDirectory == FilesystemMountPriority::LOWER &&
				   options.mountPriorityOfArchiveSearchRelativeToDataDirectory == FilesystemMountPriority::HIGHER) {
			mountArchives(options.archiveSearchPath, options.archiveSearchFileExtension, FilesystemMountPriority::HIGHER);
			if (!outputDirectory.empty()) {
				if (options.mountPriorityOfDataDirectoryRelativeToOutputDirectory == FilesystemMountPriority::HIGHER) {
					throw File::Error{"Conflicting mount priorities specified in initial filesystem configuration."};
				}
				unmountArchive(outputDirectory.c_str());
				mountArchive(outputDirectory.c_str(), FilesystemMountPriority::HIGHER);
			}
		} else if (options.mountPriorityOfArchiveSearchRelativeToOutputDirectory == FilesystemMountPriority::HIGHER &&
				   options.mountPriorityOfArchiveSearchRelativeToDataDirectory == FilesystemMountPriority::LOWER) {
			mountArchives(options.archiveSearchPath, options.archiveSearchFileExtension, FilesystemMountPriority::HIGHER);
			if (options.dataDirectory) {
				if (options.mountPriorityOfDataDirectoryRelativeToOutputDirectory == FilesystemMountPriority::LOWER) {
					throw File::Error{"Conflicting mount priorities specified in initial filesystem configuration."};
				}
				unmountArchive(options.dataDirectory);
				mountArchive(options.dataDirectory, FilesystemMountPriority::HIGHER);
			}
		} else {
			mountArchives(options.archiveSearchPath, options.archiveSearchFileExtension, FilesystemMountPriority::HIGHER);
		}
	}
}

Filesystem::~Filesystem() {
	PHYSFS_deinit();
}

std::string Filesystem::getStandardOutputDirectory(const char* organizationName, const char* applicationName) const {
	assert(organizationName);
	assert(applicationName);
	const char* const preferencesDirectory = PHYSFS_getPrefDir(organizationName, applicationName);
	return (preferencesDirectory) ? std::string{preferencesDirectory} : std::string{};
}

std::string Filesystem::getOutputDirectory() const {
	return outputDirectory;
}

void Filesystem::setOutputDirectory(std::string path) {
	if (PHYSFS_setWriteDir(path.c_str()) == 0) {
		throw File::Error{std::format("Failed to set output directory:\n{}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
	outputDirectory = std::move(path);
}

void Filesystem::mountArchive(const char* path, FilesystemMountPriority priority) {
	assert(path);
	if (PHYSFS_mount(path, nullptr, (priority == FilesystemMountPriority::LOWER) ? 1 : 0) == 0) {
		throw File::Error{std::format("Failed to mount archive \"{}\":\n{}\n", path, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
}

void Filesystem::unmountArchive(const char* path) {
	assert(path);
	if (PHYSFS_unmount(path) == 0) {
		throw File::Error{std::format("Failed to unmount archive \"{}\":\n{}\n", path, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
}

std::vector<std::string> Filesystem::mountArchives(const char* filepath, const char* archiveFileExtension, FilesystemMountPriority priority) {
	assert(filepath);
	struct Context {
		Filesystem& filesystem;
		const char* const archiveFileExtension;
		FilesystemMountPriority priority;
		std::vector<std::string> mountedFilepaths{};
	} context{
		.filesystem = *this,
		.archiveFileExtension = archiveFileExtension,
		.priority = priority,
	};
	PHYSFS_enumerate(
		filepath,
		[](void* data, const char*, const char* fname) -> PHYSFS_EnumerateCallbackResult {
			Context& context = *static_cast<Context*>(data);
			const std::string_view filename{fname};
			if (context.archiveFileExtension) {
				if (const std::string_view extension{context.archiveFileExtension};
					filename.size() <= extension.size() || (context.archiveFileExtension[0] != '.' && filename[filename.size() - extension.size() - 1] != '.') ||
					filename.ends_with(extension)) {
					return PHYSFS_ENUM_OK;
				}
			}
			const char* directory = PHYSFS_getRealDir(fname);
			if (!directory) {
				throw File::Error{std::format("Failed to get the real directory of archive \"{}\": {}\n", filename, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
			}
			std::string archiveFilepath = std::format("{}{}{}", directory, PHYSFS_getDirSeparator(), fname);
			context.filesystem.mountArchive(archiveFilepath.c_str(), context.priority);
			context.mountedFilepaths.push_back(std::move(archiveFilepath));
			return PHYSFS_ENUM_OK;
		},
		&context);
	return std::move(context.mountedFilepaths);
}

void Filesystem::unmountArchives(std::span<const std::string> paths) {
	for (const std::string& path : paths) {
		unmountArchive(path.c_str());
	}
}

const char* Filesystem::findArchiveOfFile(const char* filepath) const {
	assert(filepath);
	return PHYSFS_getRealDir(filepath);
}

void Filesystem::createDirectory(const char* filepath) {
	assert(filepath);
	if (PHYSFS_mkdir(filepath) == 0) {
		throw File::Error{std::format("Failed to create directory \"{}\": {}", filepath, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
}

void Filesystem::deleteFile(const char* filepath) {
	assert(filepath);
	if (PHYSFS_delete(filepath) == 0) {
		throw File::Error{std::format("Failed to delete file \"{}\": {}", filepath, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
}

bool Filesystem::fileExists(const char* filepath) const {
	assert(filepath);
	return PHYSFS_exists(filepath) != 0;
}

File::Metadata Filesystem::getFileMetadata(const char* filepath) const {
	assert(filepath);
	PHYSFS_Stat metadata{};
	if (PHYSFS_stat(filepath, &metadata) == 0) {
		throw File::Error{std::format("Failed to get file metadata: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
	File::Kind kind{};
	switch (metadata.filetype) {
		case PHYSFS_FILETYPE_REGULAR: kind = File::Kind::REGULAR; break;
		case PHYSFS_FILETYPE_DIRECTORY: kind = File::Kind::DIRECTORY; break;
		case PHYSFS_FILETYPE_SYMLINK: kind = File::Kind::SYMLINK; break;
		case PHYSFS_FILETYPE_OTHER: kind = File::Kind::OTHER; break;
	}
	return {
		.size = (metadata.filesize < 0) ? File::NPOS : static_cast<std::size_t>(metadata.filesize),
		.creationTime = metadata.createtime,
		.lastAccessTime = metadata.accesstime,
		.lastModificationTime = metadata.modtime,
		.kind = kind,
		.readOnly = metadata.readonly != 0,
	};
}

std::vector<std::string> Filesystem::getFilenamesInDirectory(const char* filepath) const {
	assert(filepath);
	std::vector<std::string> result{};
	if (PHYSFS_enumerate(
			filepath,
			[](void* data, const char*, const char* fname) -> PHYSFS_EnumerateCallbackResult {
				static_cast<std::vector<std::string>*>(data)->emplace_back(fname);
				return PHYSFS_ENUM_OK;
			},
			&result) == 0) {
		throw File::Error{std::format("Failed to enumerate directory: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
	return result;
}

File Filesystem::openFile(const char* filepath) const {
	assert(filepath);
	File result{PHYSFS_openRead(filepath)};
	if (!result) {
		throw File::Error{std::format("Failed to open file \"{}\" for reading: {}", filepath, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
	return result;
}

File Filesystem::createFile(const char* filepath) {
	assert(filepath);
	File result{PHYSFS_openWrite(filepath)};
	if (!result) {
		throw File::Error{std::format("Failed to create file \"{}\" for writing: {}", filepath, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
	return result;
}

File Filesystem::appendFile(const char* filepath) {
	assert(filepath);
	File result{PHYSFS_openAppend(filepath)};
	if (!result) {
		throw File::Error{std::format("Failed to open file \"{}\" for appending: {}", filepath, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
	}
	return result;
}

} // namespace donut
