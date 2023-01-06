#ifndef DONUT_INPUT_FILE_STREAM_HPP
#define DONUT_INPUT_FILE_STREAM_HPP

#include <donut/File.hpp>

#include <cstddef> // std::size_t, std::ptrdiff_t, std::byte
#include <span>    // std::span
#include <string>  // std::string
#include <vector>  // std::vector

namespace donut {

/**
 * Input stream wrapping a virtual file handle for reading the contents of the
 * associated file.
 *
 * \sa File
 * \sa OutputFileStream
 */
class InputFileStream : public File {
public:
	/**
	 * Open a file in the virtual filesystem for reading.
	 *
	 * \param filepath virtual filepath of the file to open.
	 *
	 * \return a new virtual file handle with an input stream set up to read the
	 *         opened file starting at file position 0.
	 *
	 * \throws File::Error on failure to open the file for reading.
	 * \throws std::bad_alloc on allocation failure.
	 */
	[[nodiscard]] static InputFileStream open(const char* filepath);

	/**
	 * Construct a closed virtual file handle without an associated file.
	 */
	constexpr InputFileStream() noexcept = default;

	/**
	 * Read the full contents of the open file into an array of bytes.
	 *
	 * \return a contiguous container containing a copy of the full contents of
	 *         the file.
	 *
	 * \throws File::Error on failure to read the file contents.
	 * \throws std::bad_alloc on allocation failure.
	 */
	[[nodiscard]] std::vector<std::byte> readAll() &&;

	/**
	 * Read the full contents of the open file into a string of bytes.
	 *
	 * \return a string containing a copy of the full contents of the file.
	 *
	 * \throws File::Error on failure to read the file contents.
	 * \throws std::bad_alloc on allocation failure.
	 */
	[[nodiscard]] std::string readAllIntoString() &&;

	/**
	 * Get the readable length of the full file contents, in bytes.
	 *
	 * \return the length of the file, or File::NPOS if the length cannot be
	 *         determined.
	 */
	[[nodiscard]] std::size_t size() const noexcept;

	/**
	 * Get the current file reading position of the input stream.
	 *
	 * \return the current file position, or File::NPOS if it cannot be
	 *         determined.
	 */
	[[nodiscard]] std::size_t tell() const noexcept;

	/**
	 * Read data from the open file into a buffer, starting at the current
	 * reading position of the input stream.
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
	 * \note If reading fails entirely, the function returns 0.
	 */
	[[nodiscard]] std::size_t read(std::span<std::byte> data) noexcept;

	/**
	 * Set the file reading position of the input stream to an absolute offset
	 * from the beginning of the file.
	 *
	 * \param position the new reading position to set for the input stream.
	 *
	 * \return true if the reading position was set successfully, false
	 *         otherwise.
	 *
	 * \note Attempting to seek to a position outside the readable length of the
	 *       file will cause the function to fail and return false.
	 */
	bool seek(std::size_t position) noexcept;

	/**
	 * Advance the file reading position of the input stream by a relative
	 * offset, which may be negative to go backwards.
	 *
	 * \param n the relative offset to advacne the reading position of the input
	 *        stream by.
	 *
	 * \return true if the reading position was set successfully, false
	 *         otherwise.
	 *
	 * \note Attempting to seek to a position outside the readable length of the
	 *       file will cause the function to fail and return false.
	 */
	bool skip(std::ptrdiff_t n) noexcept;

	/**
	 * Check if the input stream has reached the end of the file.
	 *
	 * \return true if the end of the file has been reached, false otherwise.
	 */
	[[nodiscard]] bool eof() const noexcept;

private:
	constexpr explicit InputFileStream(void* handle) noexcept
		: File(handle) {}
};

} // namespace donut

#endif
