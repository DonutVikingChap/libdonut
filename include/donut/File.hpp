#ifndef DONUT_FILE_HPP
#define DONUT_FILE_HPP

#include <donut/Resource.hpp>

#include <cstddef>   // std::size_t
#include <stdexcept> // std::runtime_error
#include <string>    // std::string

namespace donut {

/**
 * Abstract virtual file handle.
 *
 * The structure of the virtual file system is determined by the configuration
 * of the application::ApplicationOptions that were passed to the running
 * application::Application. This system allows for mounting multiple physical
 * directories to the same virtual mount point, mapping each contained file
 * path to the corresponding physical file with the highest mount priority for
 * the purposes of reading. For writing, the application defines a specific
 * centralized directory known as the application folder, where any output files
 * will be written. See the documentation of application::ApplicationOptions for
 * more information.
 *
 * \warning This virtual File handle type and all of its derived types may only
 *          be used during the lifetime of an application::Application, which
 *          initializes the relevant global context upon construction.
 *          Attempting to use the File API without an active application results
 *          in undefined behavior. Assuming the application is created correctly
 *          in the main function of the program, the main case that could cause
 *          this to happen is when initializing global or static variables,
 *          whose initializers may execute before main is called.
 *
 * \sa InputFileStream
 * \sa OutputFileStream
 */
class File {
public:
	/**
	 * Exception type for errors that may arise when attempting to access
	 * files through the virtual File API.
	 */
	struct Error : std::runtime_error {
		explicit Error(const std::string& message)
			: std::runtime_error(message) {}

		explicit Error(const char* message)
			: std::runtime_error(message) {}
	};

	/**
	 * Invalid value for a file offset, used as an end-of-file marker.
	 */
	static constexpr std::size_t NPOS = static_cast<std::size_t>(-1);

	/**
	 * Create a new physical directory in the write directory.
	 *
	 * \param filepath virtual filepath, relative to the application folder,
	 *        of the new directory to be created.
	 *
	 * \throws Error on failure to create the given directory.
	 * \throws std::bad_alloc on allocation failure.
	 */
	static void createDirectory(const char* filepath);

	/**
	 * Delete a physical file or directory in the write directory.
	 *
	 * \param filepath virtual filepath, relative to the application folder,
	 *        of the file or directory to delete.
	 *
	 * \throws Error on failure to delete the given file or directory.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \warning If successful, this will delete the physical file that
	 *          corresponds to the given virtual filepath on the host
	 *          filesystem; not just the virtual file entry.
	 * \warning Althrough deleting a file will prevent it from being read again
	 *          through conventional means, the physical data that was contained
	 *          in the file may or may not remain untouched on disk, meaning
	 *          that this function cannot be relied upon to securly erase
	 *          sensitive data.
	 *
	 * \note Directories must be empty before they can be deleted with this
	 *       function.
	 */
	static void deleteFile(const char* filepath);

	/**
	 * Check if a given virtual filepath has a corresponding physical
	 * file mounted.
	 *
	 * \param filepath virtual filepath to check for a mounted file.
	 *
	 * \return true if a file is mounted at the given filepath, false otherwise.
	 */
	[[nodiscard]] static bool exists(const char* filepath);

	/**
	 * Construct a closed virtual file handle without an associated file.
	 */
	constexpr File() noexcept = default;

	/**
	 * Virtual destructor.
	 *
	 * Closes the associated file if open.
	 *
	 * \warning The destructor does not report any potential failures to close
	 *          the file. The close() function can be used before destroying the
	 *          file handle in order to make sure the file closed successfully.
	 *
	 * \sa close()
	 */
	virtual ~File() = default;

	/**
	 * Copying a file handle is not allowed, since each handle has the exclusive
	 * responsibility of closing its associated file.
	 */
	File(const File&) = delete;

	/** Move constructor. */
	constexpr File(File&& other) noexcept = default;

	/**
	 * Copying a file handle is not allowed, since each handle has the exclusive
	 * responsibility of closing its associated file.
	 */
	File& operator=(const File&) = delete;

	/** Move assignment. */
	File& operator=(File&& other) noexcept = default;

	/**
	 * Check if the file handle has an open file associated with it.
	 *
	 * \return true if there is an associated open file, false otherwise.
	 */
	constexpr explicit operator bool() const noexcept {
		return static_cast<bool>(file);
	}

	/**
	 * Close the associated file so that it can no longer be accessed through
	 * this handle, and reset the handle to a closed virtual file handle without
	 * an associated file.
	 *
	 * \throws Error on failure to close the file.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note This function has no effect if the handle has no open file
	 *       associated with it.
	 *
	 * \sa ~File()
	 */
	void close();

private:
	struct FileDeleter {
		void operator()(void* handle) const noexcept;
	};

protected:
	/**
	 * Construct a virtual file handle from an underlying handle pointer.
	 *
	 * \param handle the underlying handle pointer to take ownership of.
	 */
	constexpr explicit File(void* handle) noexcept
		: file(handle) {}

	/**
	 * Resource handle wrapping the underlying handle pointer to the virtual
	 * file.
	 */
	Resource<void*, FileDeleter, nullptr> file{};
};

} // namespace donut

#endif
