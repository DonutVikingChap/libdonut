#ifndef DONUT_RESOURCE_HPP
#define DONUT_RESOURCE_HPP

#include <utility>

namespace donut {

template <typename Handle, typename Deleter, Handle NullHandle = Handle{}>
class Resource {
public:
	constexpr explicit Resource(Handle handle = NullHandle) noexcept
		: handle(handle) {}

	~Resource() {
		Deleter{}(handle);
	}

	Resource(const Resource&) = delete;

	constexpr Resource(Resource&& other) noexcept
		: handle(other.release()) {}

	Resource& operator=(const Resource&) = delete;

	constexpr Resource& operator=(Resource&& other) noexcept {
		reset(other.release());
		return *this;
	}

	[[nodiscard]] constexpr explicit operator bool() const noexcept {
		return get() != NullHandle;
	}

	[[nodiscard]] friend constexpr bool operator==(const Resource& a, const Resource& b) noexcept {
		return a.get() == b.get();
	}

	[[nodiscard]] friend constexpr bool operator!=(const Resource& a, const Resource& b) noexcept {
		return a.get() != b.get();
	}

	constexpr void reset(Handle newHandle = NullHandle) noexcept {
		Deleter{}(std::exchange(handle, newHandle));
	}

	constexpr Handle release() noexcept {
		return std::exchange(handle, NullHandle);
	}

	[[nodiscard]] constexpr Handle get() const noexcept {
		return handle;
	}

private:
	Handle handle;
};

} // namespace donut

#endif
