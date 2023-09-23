#ifndef DONUT_FILESYSTEM_HPP
#define DONUT_FILESYSTEM_HPP

#include <donut/File.hpp>

#include <span>   // std::span
#include <string> // std::string
#include <vector> // std::vector

namespace donut {

/**
 * Mount priority for a newly mounted archive to a virtual Filesystem, relative
 * to all previously mounted archives.
 */
enum class FilesystemMountPriority {
	/**
	  * Mount the archive at a lower priority than any previously mounted
	  * archive, meaning files in already mounted archives will be preferred
	  * when choosing which host file to map a virtual filepath to.
	  */
	LOWER,

	/**
	 * Mount the archive at a higher priority than any previously mounted
	 * archive, meaning files in the new archive will be preferred when
	 * choosing which host file to map a virtual filepath to.
	 */
	HIGHER,
};

/**
 * Configuration options for a virtual Filesystem.
 */
struct FilesystemOptions {
	/**
	 * Non-owning pointer to a null-terminated UTF-8 string that commonly
	 * identifies the publisher of the application, such as an organization
	 * name, alias or internet domain.
	 *
	 * When set, this is used to determine part of the filesystem's initial
	 * output directory, into which files such as configuration or save data can
	 * then be written by the application. Namely, it is used for the name of
	 * the organization folder in the user/platform-specific preferences
	 * directory on platforms where it is applicable, which will be created if
	 * it doesn't already exist.
	 *
	 * If set to nullptr, no output directory will be created nor mounted, and
	 * the application will be unable to write files unless an output directory
	 * is set manually using Filesystem::setOutputDirectory().
	 *
	 * \sa applicationName
	 * \sa mountOutputDirectory
	 * \sa outputDirectoryMountPriority
	 * \sa dataDirectory
	 */
	const char* organizationName = nullptr;

	/**
	 * Non-owning pointer to a null-terminated UTF-8 string that uniquely
	 * identifies the application among all other applications released by the
	 * same organization.
	 *
	 * When set, this is used to determine part of the filesystem's initial
	 * write directory, into which files such as configuration or save data can
	 * then be written by the application. Namely, it is used for the name of
	 * the application folder under the organization folder in the
	 * user/platform-specific preferences directory on platforms where it is
	 * applicable, which will be created if it doesn't already exist.
	 *
	 * If set to nullptr, no output directory will be created nor mounted, and
	 * the application will be unable to write files unless an output directory
	 * is set manually using Filesystem::setOutputDirectory().
	 *
	 * \sa organizationName
	 * \sa mountOutputDirectory
	 * \sa outputDirectoryMountPriority
	 * \sa dataDirectory
	 */
	const char* applicationName = nullptr;

	/**
	 * Non-owning pointer to a null-terminated UTF-8 string of the host filepath
	 * to the main data directory which will be mounted for reading.
	 *
	 * If set to nullptr, no main data directory will be mounted, and the
	 * application will be unable to read any files unless the output directory
	 * or additional archives are mounted, or if an archive is mounted manually
	 * using Filesystem::mountArchive().
	 *
	 * \sa organizationName
	 * \sa applicationName
	 */
	const char* dataDirectory = ".";

	/**
	 * Non-owning pointer to a null-terminated UTF-8 string of the virtual
	 * filepath to a directory in which to search for additional initial
	 * archives to mount.
	 *
	 * If set to nullptr, no additional archives will be mounted.
	 *
	 * \sa archiveSearchFileExtension
	 * \sa archiveSearchMountPriority
	 */
	const char* archiveSearchPath = nullptr;

	/**
	 * Non-owning pointer to a null-terminated UTF-8 string of the filename
	 * extension of initial archives to search for.
	 *
	 * If set to nullptr, all found archives will be mounted regardless of
	 * extension.
	 *
	 * \note This option is only applicable when #archiveSearchPath is not
	 *	   nullptr.
	 *
	 * \sa archiveSearchPath
	 * \sa archiveSearchMountPriority
	 */
	const char* archiveSearchFileExtension = nullptr;

	/**
	 * Mount priority of the main data directory relative to the initial write
	 * directory.
	 *
	 * \note This option is only applicable when #mountOutputDirectory
	 *	   is set to true, and neither #dataDirectory, #organizationName nor
	 *	   #applicationName are nullptr.
	 *
	 * \sa organizationName
	 * \sa applicationName
	 * \sa mountOutputDirectory
	 */
	FilesystemMountPriority mountPriorityOfDataDirectoryRelativeToOutputDirectory = FilesystemMountPriority::LOWER;

	/**
	 * Mount priority of the additional initial archives relative to the initial
	 * write directory.
	 *
	 * \note This option is only applicable when #archiveSearchPath is not
	 *	   nullptr.
	 *
	 * \sa archiveSearchPath
	 * \sa archiveSearchFileExtension
	 */
	FilesystemMountPriority mountPriorityOfArchiveSearchRelativeToOutputDirectory = FilesystemMountPriority::LOWER;

	/**
	 * Mount priority of the additional initial archives relative to the main
	 * data directory.
	 *
	 * \note This option is only applicable when #archiveSearchPath is not
	 *	   nullptr.
	 *
	 * \sa archiveSearchPath
	 * \sa archiveSearchFileExtension
	 */
	FilesystemMountPriority mountPriorityOfArchiveSearchRelativeToDataDirectory = FilesystemMountPriority::HIGHER;

	/**
	 * Mount the initial output directory for reading in addition to writing.
	 *
	 * \note This option is only applicable when neither #organizationName nor
	 *	   #applicationName are nullptr.
	 *
	 * \sa organizationName
	 * \sa applicationName
	 * \sa outputDirectoryMountPriority
	 * \sa dataDirectory
	 */
	bool mountOutputDirectory = true;
};

/**
 * Persistent system for managing the virtual filesystem.
 *
 * This system allows for mounting multiple host filesystem directories to the
 * same virtual mount point, mapping each contained file path to the
 * corresponding host file with the highest mount priority for the purposes of
 * reading. For writing, the filesystem defines a specific centralized folder
 * known as the output directory, where any output files will be written. See
 * FilesystemOptions for more information.
 */
class Filesystem {
public:
	/**
	 * Initialize the virtual filesystem.
	 *
	 * \param programFilepath the first string in the argument vector passed to
	 *		the main function of the program, i.e. argv[0].
	 * \param options initial configuration of the virtual filesystem, see
	 *		FilesystemOptions.
	 *
	 * \throws File::Error if initialization failed.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \warning The behavior of passing programFilepath a value other than the
	 *		  argv[0] string received from main is undefined.
	 * \warning There can only be one active virtual filesystem in a program at
	 *		  a time.
	 *
	 * \sa setOutputDirectory()
	 * \sa mountArchives()
	 * \sa mountArchive()
	 */
	explicit Filesystem(const char* programFilepath, const FilesystemOptions& options = {});

	/** Destructor. */
	~Filesystem();

	/** Copying a filesystem is not allowed, since it manages global state. */
	Filesystem(const Filesystem&) = delete;

	/** Moving a filesystem is not allowed, since it manages global state. */
	Filesystem(Filesystem&&) = delete;

	/** Copying a filesystem is not allowed, since it manages global state. */
	Filesystem& operator=(const Filesystem&) = delete;

	/** Moving a filesystem is not allowed, since it manages global state. */
	Filesystem& operator=(Filesystem&&) = delete;

	/**
	 * Get a suitable output directory for configuration files and other save
	 * data on the host platform, which is usually located somwhere within the
	 * user's home directory in a sub-folder tailored to this specific
	 * application.
	 *
	 * \param organizationName non-owning pointer to a null-terminated UTF-8
	 *		string that commonly identifies the publisher of the application,
	 *		such as an organization name, alias or internet domain. This will
	 *		be used for the name of the organization folder in the
	 *		user/platform-specific preferences directory on platforms where it
	 *		is applicable. Must not be nullptr.
	 * \param applicationName non-owning pointer to a null-terminated UTF-8
	 *		string that uniquely identifies the application among all other
	 *		applications released by the same organization. This will be used
	 *		for the name of the application folder under the organization
	 *		folder in the user/platform-specific preferences directory on
	 *		platforms where it is applicable. Must not be nullptr.
	 *
	 * \return a directory that can be passed to setOutputDirectory(), or an
	 *		 empty string if a suitable output directory could not be
	 *		 determined.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \sa setOutputDirectory()
	 * \sa getOutputDirectory()
	 */
	[[nodiscard]] std::string getStandardOutputDirectory(const char* organizationName, const char* applicationName) const;

	/**
	 * Get the current output directory of the virtual filesystem.
	 *
	 * \return the host filepath corresponding to the current output directory,
	 *		 or an empty string if no output directory is currently set.
	 *
	 * \sa getStandardOutputDirectory()
	 * \sa setOutputDirectory()
	 */
	[[nodiscard]] std::string getOutputDirectory() const;

	/**
	 * Set the output directory of the virtual filesystem, where all output
	 * files will be written.
	 *
	 * The specified directory will be created if it doesn't already exist.
	 *
	 * \param path host filepath of the new output directory to set.
	 *
	 * \throws File::Error on failure to set the output directory to the given
	 *		 path.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note To be able to read files from the output directory, it must also be
	 *	   mounted using mountArchive().
	 *
	 * \sa getStandardOutputDirectory()
	 * \sa getOutputDirectory()
	 */
	void setOutputDirectory(std::string path);

	/**
	 * Mount a directory or archive on the host filesystem to the root directory
	 * of the virtual filesystem.
	 *
	 * \param path host filepath of the directory or archive to mount. Must not
	 *		be nullptr.
	 * \param priority mount priority of the archive to be mounted, see
	 *		FilesystemMountPriority.
	 *
	 * \throws File::Error on failure to mount the given filepath.
	 *
	 * \note If the given path is already mounted, no change will occur.
	 *
	 * \sa mountArchives()
	 * \sa unmountArchive()
	 */
	void mountArchive(const char* path, FilesystemMountPriority priority = FilesystemMountPriority::HIGHER);

	/**
	 * Unmount a previously mounted directory or archive on the host filesystem
	 * from the virtual filesystem.
	 *
	 * \param path host filepath of the directory or archive to unmount, which
	 *		was previously mounted by mountArchive() or mountArchives(). Must
	 *		not be nullptr.
	 *
	 * \throws File::Error on failure to unmount the given filepath.
	 *
	 * \sa mountArchive()
	 */
	void unmountArchive(const char* path);

	/**
	 * Mount all archives in a given directory on the host filesystem to the
	 * root directory of the virtual filesystem. This is useful for allowing
	 * users to easily create and share modifications or plugins that add or
	 * override application resources by simply adding the mod archive to the
	 * given directory.
	 *
	 * The newly mounted archives will have a higher priority than any
	 * previously mounted archives when choosing which host file to map a
	 * virtual filepath to, meaning more recently mounted files are preferred.
	 *
	 * \param filepath virtual filepath of the mounted directory to search for
	 *		archives in. Must not be nullptr.
	 * \param archiveFileExtension non-owning pointer to a null-terminated UTF-8
	 *		string of the filename extension of the archive files to mount. If
	 *		set to nullptr, all found archives will be mounted regardless of
	 *		extension.
	 * \param priority mount priority of the archives to be mounted, see
	 *		FilesystemMountPriority.
	 *
	 * \return an input-iterable sequence containing the host filepaths of the
	 *		 archives that were mounted, in no specific order.
	 *
	 * \throws File::Error on failure to search for archives or mount an
	 *		 archive.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note The found archives are mounted in no particular order.
	 *
	 * \sa unmountArchives()
	 * \sa mountArchive()
	 * \sa unmountArchive()
	 */
	std::vector<std::string> mountArchives(const char* filepath, const char* archiveFileExtension, FilesystemMountPriority priority = FilesystemMountPriority::HIGHER);

	/**
	 * Unmount multiple archives.
	 *
	 * Equivalent to calling unmountArchive() on each given path.
	 *
	 * \param paths the host filepaths of the archives to unmount, which
	 *		were previously mounted by mountArchive() or mountArchives().
	 *
	 * \throws File::Error on failure to unmount a given filepath.
	 *
	 * \sa mountArchives()
	 * \sa mountArchive()
	 * \sa unmountArchives()
	 */
	void unmountArchives(std::span<const std::string> paths);

	/**
	 * Get the host filepath of the mounted archive on the host filesystem that
	 * contains a given virtual file.
	 *
	 * \param filepath virtual filepath of the file to get the archive of. Must
	 *		not be nullptr.
	 *
	 * \return the host filepath of the directory or archive that was previously
	 *		 passed to mountArchive(), in which the corresponding virtual file
	 *		 was found, or nullptr if the virtual file was not found at any
	 *		 active mount point.
	 *
	 * \warning This function may return nullptr.
	 *
	 * \sa mountArchive()
	 */
	[[nodiscard]] const char* findArchiveOfFile(const char* filepath) const;

	/**
	 * Create a new host directory in the output directory.
	 *
	 * \param filepath virtual filepath, relative to the current output
	 *		directory, of the new directory to be created. Must not be
	 *		nullptr.
	 *
	 * \throws Error on failure to create the given directory.
	 * \throws std::bad_alloc on allocation failure.
	 */
	void createDirectory(const char* filepath);

	/**
	 * Delete a host file or directory in the output directory.
	 *
	 * \param filepath virtual filepath, relative to the current output
	 *		directory, of the file or directory to delete. Must not be
	 *		nullptr.
	 *
	 * \throws Error on failure to delete the given file or directory.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \warning If successful, this will delete the actual file that
	 *		  corresponds to the given virtual filepath on the host
	 *		  filesystem; not just the virtual file entry.
	 * \warning Although deleting a file will prevent it from being read again
	 *		  through conventional means, the physical data that was contained
	 *		  in the file may or may not remain untouched on disk, meaning
	 *		  that this function cannot be relied upon to securly erase
	 *		  sensitive data.
	 *
	 * \note Directories must be empty before they can be successfully deleted
	 *	   using this function.
	 */
	void deleteFile(const char* filepath);

	/**
	 * Check if a given virtual filepath has a corresponding host file mounted.
	 *
	 * \param filepath virtual filepath to check for a mounted file. Must not be
	 *		nullptr.
	 *
	 * \return true if a file is mounted at the given filepath, false otherwise.
	 *
	 * \sa getFileMetadata()
	 * \sa getFilenamesInDirectory()
	 */
	[[nodiscard]] bool fileExists(const char* filepath) const;

	/**
	 * Get the metadata of a file that is mounted at a given virtual filepath.
	 *
	 * \param filepath virtual filepath of the file to get the metadata of. Must
	 *		not be nullptr.
	 *
	 * \return the file metadata, see File::Metadata.
	 *
	 * \throws File::Error on failure to get the metadata, such as if no file is
	 *		 mounted at the given filepath.
	 * \throws std::bad_alloc on allocation failure.
	 */
	[[nodiscard]] File::Metadata getFileMetadata(const char* filepath) const;

	/**
	 * Get a list of the filenames of all readable virtual filepaths that are
	 * direct children of a given directory.
	 *
	 * \param filepath virtual filepath of the directory to enumerate. Must not
	 *		be nullptr.
	 *
	 * \return an input-iterable sequence containing the filenames, in no
	 *		 specific order.
	 *
	 * \throws Error on failure to enumerate the directory.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note This function is not recursive, and only returns the filename
	 *	   component of the direct descendants of the given directory, without
	 *	   the leading directory path. The full virtual filepath of each
	 *	   result can be formatted as `"{filepath}/{filename}"`, where
	 *	   `{filepath}` is the directory filepath that was passed to the
	 *	   function, and `{filename}` is one of the results in the returned
	 *	   sequence. Note that this path may refer to any kind of file,
	 *	   including a subdirectory. Use getFileMetadata() to find out which
	 *	   kind of file it refers to.
	 *
	 * \sa fileExists()
	 * \sa getFileMetadata()
	 */
	[[nodiscard]] std::vector<std::string> getFilenamesInDirectory(const char* filepath) const;

	/**
	 * Open a file in the virtual filesystem for reading.
	 *
	 * \param filepath virtual filepath of the file to open. Must not be
	 *		nullptr.
	 *
	 * \return a new virtual file handle with an input stream set up to read the
	 *		 opened file starting at file position 0.
	 *
	 * \throws File::Error on failure to open the file for reading.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \sa createFile()
	 * \sa appendFile()
	 */
	[[nodiscard]] File openFile(const char* filepath) const;

	/**
	 * Create a file in the output directory and open it for writing,
	 * overwriting any existing file at the same filepath.
	 *
	 * \param filepath virtual filepath of the new file to be created, relative
	 *		to the current output directory. Must not be nullptr.
	 *
	 * \return a new virtual file handle with an output stream set up to write
	 *		 to the new empty file that was opened.
	 *
	 * \throws File::Error on failure to delete the existing file, create the
	 *		 new file, or open the new file for writing.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \warning If successful, any existing host file that corresponds to the
	 *		  given virtual filepath in the output directory on the host
	 *		  filesystem will be deleted on the host filesystem; not just its
	 *		  virtual file entry. All modifications made through the virtual
	 *		  file handle will also be reflected in the physical file on the
	 *		  host file system.
	 *
	 * \sa openFile()
	 * \sa appendFile()
	 */
	[[nodiscard]] File createFile(const char* filepath);

	/**
	 * Open a file in the output directory specified by the virtual filesystem
	 * for appended writing.
	 *
	 * \param filepath virtual filepath of the file to be opened, relative to
	 *		the current output directory. Must not be nullptr.
	 *
	 * \return a new virtual file handle with an output stream set up to write
	 *		 to the end of the opened file.
	 *
	 * \throws File::Error on failure to open the file for writing.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \warning If successful, the existing host file that corresponds to the
	 *		  given virtual filepath in the output directory on the host
	 *		  filesystem will receive all modifications made through the
	 *		  virtual file handle.
	 *
	 * \note A new, empty file will be created if the specified file doesn't
	 *	   already exist.
	 *
	 * \sa openFile()
	 * \sa createFile()
	 */
	[[nodiscard]] File appendFile(const char* filepath);

private:
	std::string outputDirectory{};
};

} // namespace donut

#endif
