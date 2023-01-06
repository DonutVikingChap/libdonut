#ifndef DONUT_OUTPUT_FILE_STREAM_HPP
#define DONUT_OUTPUT_FILE_STREAM_HPP

#include <donut/File.hpp>

#include <cstddef> // std::size_t, std::byte
#include <span>    // std::span

namespace donut {

/**
 * Output stream wrapping a virtual file handle for writing to the contents of
 * the associated file.
 *
 * \sa File
 * \sa InputFileStream
 */
class OutputFileStream : public File {
public:
	/**
	 * Create a file in the write directory and open it for writing,
	 * overwriting any existing file at the same filepath.
	 *
	 * \param filepath virtual filepath, relative to the application folder,
	 *        of the new file to be created.
	 *
	 * \return a new virtual file handle with an output stream set up to write
	 *         to the new empty file that was opened.
	 *
	 * \throws File::Error on failure to delete the existing file, create the
	 *         new file, or open the new file for writing.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \warning If successful, any existing physical file that corresponds to
	 *          the given virtual filepath in the write directory on the host
	 *          filesystem will be deleted on the host filesystem; not just its
	 *          virtual file entry. All modifications made through the virtual
	 *          file handle will also be reflected in the physical file on the
	 *          host file system.
	 */
	[[nodiscard]] static OutputFileStream create(const char* filepath);

	/**
	 * Open a file in the write directory for writing.
	 *
	 * \param filepath virtual filepath, relative to the application folder,
	 *        of the file to be opened.
	 *
	 * \return a new virtual file handle with an output stream set up to write
	 *         to the end of the opened file.
	 *
	 * \throws File::Error on failure to open the file for writing.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \warning If successful, the existing physical file that corresponds to
	 *          the given virtual filepath in the write directory on the host
	 *          filesystem will receive all modifications made through the
	 *          virtual file handle.
	 *
	 * \note A new, empty file will be created if the specified file doesn't
	 *       already exist.
	 */
	[[nodiscard]] static OutputFileStream append(const char* filepath);

	/**
	 * Construct a closed virtual file handle without an associated file.
	 */
	constexpr OutputFileStream() noexcept = default;

	/**
	 * Write data to the end of the open file from a buffer.
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
	 * \note If writing fails entirely, the function returns 0.
	 */
	std::size_t write(std::span<const std::byte> data) noexcept;

private:
	constexpr explicit OutputFileStream(void* handle) noexcept
		: File(handle) {}
};

} // namespace donut

#endif
