#ifndef DONUT_FILE_HPP
#define DONUT_FILE_HPP

#include <donut/Resource.hpp>

#include <cstddef>   // std::size_t
#include <stdexcept> // std::runtime_error
#include <string>    // std::string

namespace donut {

class File {
public:
	struct Error : std::runtime_error {
		explicit Error(const std::string& message)
			: std::runtime_error(message) {}

		explicit Error(const char* message)
			: std::runtime_error(message) {}
	};

	static constexpr std::size_t NPOS = -1;

	static void createDirectory(const char* filepath);

	static void deleteFile(const char* filepath);

	[[nodiscard]] static bool exists(const char* filepath);

	constexpr File() noexcept = default;
	virtual ~File() = default;

	File(const File&) = delete;
	constexpr File(File&& other) noexcept = default;

	File& operator=(const File&) = delete;
	File& operator=(File&& other) noexcept = default;

	constexpr explicit operator bool() const noexcept {
		return static_cast<bool>(file);
	}

	void close();

protected:
	constexpr explicit File(void* handle) noexcept
		: file(handle) {}

	struct FileDeleter {
		void operator()(void* handle) const noexcept;
	};

	Resource<void*, FileDeleter, nullptr> file{};
};

} // namespace donut

#endif
