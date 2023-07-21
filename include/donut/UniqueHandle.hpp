#ifndef DONUT_UNIQUE_HANDLE_HPP
#define DONUT_UNIQUE_HANDLE_HPP

#include <utility> // std::exchange

namespace donut {

/**
 * Generic nullable RAII resource handle with exclusive ownership of a resource
 * that is automatically destroyed on handle destruction.
 *
 * \tparam Handle the underlying handle type. Usually an integer or pointer
 *         type.
 * \tparam Deleter stateless functor type that destroys the associated resource
 *         when default-constructed and called with a non-null handle. Passing a
 *         handle equal to NullHandle must be a no-op.
 * \tparam NullHandle constant value representing a null handle that does not
 *         have an associated resource. Usually 0 or nullptr.
 */
template <typename Handle, typename Deleter, Handle NullHandle = Handle{}>
class UniqueHandle {
public:
	/**
	 * Construct a null handle without an associated resource.
	 */
	constexpr UniqueHandle() noexcept = default;

	/**
	 * Construct a handle that takes ownership of an existing resource handle.
	 *
	 * \param handle the underlying handle to take ownership of, or NullHandle
	 *        to construct a null handle without an associated resource.
	 */
	constexpr explicit UniqueHandle(Handle handle) noexcept
		: handle(handle) {}

	/**
	 * Destroy the handle and its associated resource if it has one.
	 */
	~UniqueHandle() {
		Deleter{}(handle);
	}

	/**
	 * Copying a resource handle is disallowed to enforce exclusive ownership.
	 */
	UniqueHandle(const UniqueHandle&) = delete;

	/**
	 * Move constructor.
	 *
	 * Takes ownership of the resource associated with the other handle and sets
	 * the other handle to null.
	 *
	 * \param other resource handle to take ownership of.
	 */
	constexpr UniqueHandle(UniqueHandle&& other) noexcept
		: handle(other.release()) {}

	/**
	 * Copying a resource handle is disallowed to enforce exclusive ownership.
	 */
	UniqueHandle& operator=(const UniqueHandle&) = delete;

	/**
	 * Move assignment.
	 *
	 * Destroys the resource associated with the current handle, if any. Then
	 * takes ownership of the resource associated with the other handle and sets
	 * the other handle to null.
	 *
	 * \param other resource handle to take ownership of.
	 *
	 * \return `*this`, for chaining.
	 */
	constexpr UniqueHandle& operator=(UniqueHandle&& other) noexcept {
		reset(other.release());
		return *this;
	}

	/**
	 * Check if this handle has an associated resource, i.e. if it is not null.
	 *
	 * \return true if the handle has an associated resource, false if the
	 *         handle is null.
	 */
	[[nodiscard]] constexpr explicit operator bool() const noexcept {
		return get() != NullHandle;
	}

	/**
	 * Compare this resource handle against another for equality of the
	 * underlying handle value.
	 *
	 * \param other the handle to compare this handle to.
	 *
	 * \return true if the handles are equal, false otherwise.
	 *
	 * \note This does not compare the values of any associated resources. It
	 *       only compares the values of the handles themselves.
	 */
	[[nodiscard]] constexpr bool operator==(const UniqueHandle& other) const noexcept {
		return get() == other.get();
	}

	/**
	 * Destroy the resource associated with this handle, if any, and take
	 * ownership of a new resource handle, which may be null.
	 *
	 * \param newHandle the new underlying handle to take ownership of, or
	 *        NullHandle to reset to a null handle without an associated
	 *        resource.
	 *
	 * \sa release()
	 * \sa get()
	 */
	constexpr void reset(Handle newHandle = NullHandle) noexcept {
		Deleter{}(std::exchange(handle, newHandle));
	}

	/**
	 * Relinquish ownership of the associated resource.
	 *
	 * This handle will be reset to null, without destroying the associated
	 * resource.
	 *
	 * \return the handle to the associated resource that was released, or
	 *         NullHandle if the handle did not have an associated resource.
	 *
	 * \warning After calling this function, the associated resource will no
	 *          longer be destroyed automatically along with the handle. It
	 *          instead becomes the responsibility of the caller to ensure that
	 *          the resource is properly cleaned up. If the intent is to reset
	 *          the handle to null while destroying the associated resource in
	 *          the process, use reset() instead.
	 *
	 * \sa reset()
	 * \sa get()
	 */
	constexpr Handle release() noexcept {
		return std::exchange(handle, NullHandle);
	}

	/**
	 * Get the value of the underlying resource handle.
	 *
	 * \return a non-owning copy of the underlying resource handle value.
	 *
	 * \sa reset()
	 * \sa release()
	 */
	[[nodiscard]] constexpr Handle get() const noexcept {
		return handle;
	}

private:
	Handle handle = NullHandle;
};

} // namespace donut

#endif
