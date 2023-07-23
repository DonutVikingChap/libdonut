#ifndef DONUT_FILE_HPP
#define DONUT_FILE_HPP

#include <donut/UniqueHandle.hpp>

#include <cstddef>   // std::size_t, std::ptrdiff_t, std::byte
#include <cstdint>   // std::int64_t, std::uint8_t
#include <span>      // std::span
#include <stdexcept> // std::runtime_error
#include <string>    // std::string
#include <vector>    // std::vector

namespace donut {

class Filesystem; // Forward declaration, to avoid a circular include of Filesystem.hpp.

/**
 * Unique handle to a file in the virtual Filesystem.
 *
 * \warning This file handle type may only be used during the lifetime of a
 *          Filesystem object, which initializes the relevant global context
 *          upon construction. Attempting to use the File API without an active
 *          Filesystem results in undefined behavior.
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
	 * Virtual file entry type.
	 */
	enum class Kind : std::uint8_t {
		REGULAR,   ///< Regular file.
		DIRECTORY, ///< Directory/folder.
		SYMLINK,   ///< Symbolic link.
		OTHER,     ///< Something else, such as a network socket or a device.
	};

	/**
	 * Record of metadata for a specific virtual file.
	 */
	struct Metadata {
		std::size_t size;                  ///< File size, in bytes, or #NPOS if unavailable.
		std::int64_t creationTime;         ///< Time when the file was created, in seconds since the Unix epoch (1970-01-01 00:00), or -1 if unavailable.
		std::int64_t lastAccessTime;       ///< Last time when the file was accessed, in seconds since the Unix epoch (1970-01-01 00:00), or -1 if unavailable.
		std::int64_t lastModificationTime; ///< Last time when the file was modified, in seconds since the Unix epoch (1970-01-01 00:00), or -1 if unavailable.
		Kind kind;                         ///< Kind of file, such as regular file or directory.
		bool readOnly;                     ///< True if the file may only be opened for reading, false if it may also be opened for writing.
	};

	/**
	 * Invalid value for a file offset, used as an end-of-file marker.
	 */
	static constexpr std::size_t NPOS = static_cast<std::size_t>(-1);

	/**
	 * Construct a closed virtual file handle without an associated file.
	 */
	constexpr File() noexcept = default;

	/**
	 * Check if the file handle has an open file associated with it.
	 *
	 * \return true if there is an associated open file, false otherwise.
	 */
	constexpr explicit operator bool() const noexcept {
		return isOpen();
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

	/**
	 * Check if the file handle has an open file associated with it.
	 *
	 * \return true if there is an associated open file, false otherwise.
	 */
	[[nodiscard]] constexpr bool isOpen() const noexcept {
		return static_cast<bool>(file);
	}

	/**
	 * Check if the end of the file has been reached.
	 *
	 * \return true if the end of the file has been reached, false otherwise.
	 */
	[[nodiscard]] bool eof() const noexcept;

	/**
	 * Get the readable length of the full file contents, in bytes.
	 *
	 * \return the length of the file, or File::NPOS if the length cannot be
	 *         determined.
	 */
	[[nodiscard]] std::size_t size() const noexcept;

	/**
	 * Get the current reading position of the file.
	 *
	 * \return the current file reading position, or File::NPOS if it cannot be
	 *         determined.
	 */
	[[nodiscard]] std::size_t tellg() const noexcept;

	/**
	 * Get the current writing position of the file.
	 *
	 * \return the current file writing position, or File::NPOS if it cannot be
	 *         determined.
	 */
	[[nodiscard]] std::size_t tellp() const noexcept;

	/**
	 * Set the file reading position to an absolute offset from the beginning of
	 * the file.
	 *
	 * \param position the new reading position to set.
	 *
	 * \throws File::Error on failure to seek to the given position.
	 *
	 * \note Attempting to seek to a position outside the readable length of the
	 *       file will cause the function to fail.
	 */
	void seekg(std::size_t position);

	/**
	 * Set the file writing position to an absolute offset from the beginning of
	 * the file.
	 *
	 * \param position the new writing position to set.
	 *
	 * \throws File::Error on failure to seek to the given position.
	 *
	 * \note Attempting to seek to a position outside the readable length of the
	 *       file will cause the function to fail.
	 */
	void seekp(std::size_t position);

	/**
	 * Advance the file reading position by a relative offset, which may be
	 * negative in order to go backwards.
	 *
	 * \param offset the relative offset to advance the reading position by.
	 *
	 * \throws File::Error on failure to skip by the given offset.
	 *
	 * \note Attempting to seek to a position outside the readable length of the
	 *       file will cause the function to fail.
	 */
	void skipg(std::ptrdiff_t offset);

	/**
	 * Advance the file writing position by a relative offset, which may be
	 * negative in order to go backwards.
	 *
	 * \param offset the relative offset to advance the writing position by.
	 *
	 * \throws File::Error on failure to skip by the given offset.
	 *
	 * \note Attempting to seek to a position outside the readable length of the
	 *       file will cause the function to fail.
	 */
	void skipp(std::ptrdiff_t offset);

	/**
	 * Read the full contents of an open file into an array of bytes.
	 *
	 * \return a contiguous container containing a copy of the full contents of
	 *         the file.
	 *
	 * \throws File::Error on failure to read the file contents.
	 * \throws std::bad_alloc on allocation failure.
	 */
	[[nodiscard]] std::vector<std::byte> readAll() &&;

	/**
	 * Read the full contents of an open file into a string of bytes.
	 *
	 * \return a string containing a copy of the full contents of the file.
	 *
	 * \throws File::Error on failure to read the file contents.
	 * \throws std::bad_alloc on allocation failure.
	 */
	[[nodiscard]] std::string readAllIntoString() &&;

	/**
	 * Read data from an open file into a buffer, starting at the current
	 * reading position.
	 *
	 * The reading position is advanced to the end of the bytes that were read.
	 *
	 * \param data writable span over the buffer to read file data into. The
	 *        size of the span determines the number of bytes that are attempted
	 *        to be read.
	 *
	 * \return the number of bytes that were successfully read, which may be any
	 *         non-negative integer less than or equal to the size of the given
	 *         span, including 0.
	 *
	 * \note If the end of the file is reached before the entire span's worth of
	 *       data could be read, this will be reflected in the returned number
	 *       of read bytes.
	 *
	 * \throws File::Error on failure to read from the file.
	 */
	[[nodiscard]] std::size_t read(std::span<std::byte> data);

	/**
	 * Write data to the end of an open file from a buffer.
	 *
	 * The writing position is advanced to the new end of the file.
	 *
	 * \param data read-only view over the buffer to copy the data from. The
	 *        size of the buffer determines the number of bytes that are
	 *        attempted to be written.
	 *
	 * \return the number of bytes that were successfully written, which may be
	 *         any non-negative integer less than or equal to the size of the
	 *         given view, including 0.
	 *
	 * \throws File::Error on failure to write to the file.
	 */
	std::size_t write(std::span<const std::byte> data);

	/**
	 * Synchronize with the underlying file to make sure that all buffered data
	 * that has been written so far is flushed into the actual file.
	 *
	 * \throws File::Error on failure to synchronize with the file.
	 */
	void flush();

private:
	friend Filesystem;

	constexpr explicit File(void* handle) noexcept
		: file(handle) {}

	struct FileDeleter {
		void operator()(void* handle) const noexcept;
	};

	UniqueHandle<void*, FileDeleter> file{};
};

} // namespace donut

#endif
