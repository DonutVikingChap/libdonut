#ifndef DONUT_LINEAR_BUFFER_HPP
#define DONUT_LINEAR_BUFFER_HPP

#include <donut/LinearAllocator.hpp>

#include <algorithm>   // std::max
#include <array>       // std::array
#include <cassert>     // assert
#include <cstddef>     // std::size_t, std::byte
#include <cstdint>     // std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t
#include <cstring>     // std::memcpy
#include <functional>  // std::invoke
#include <memory>      // std::unique_ptr, std::make_unique, std::align
#include <new>         // std::launder
#include <span>        // std::span
#include <type_traits> // std::is_..._v, std::remove_..._t, std::false_type, std::true_type, std::integral_constant, std::common_type_t
#include <utility>     // std::forward, std::declval, std::...index_sequence, std::in_place_index...
#include <vector>      // std::vector

namespace donut {

template <typename... Ts>
class LinearBuffer;

namespace detail {

template <typename T, std::size_t Index, typename... Ts>
struct LinearBufferIndexImpl;

template <typename T, std::size_t Index, typename First, typename... Rest>
struct LinearBufferIndexImpl<T, Index, First, Rest...> : LinearBufferIndexImpl<T, Index + 1, Rest...> {};

template <typename T, std::size_t Index, typename... Rest>
struct LinearBufferIndexImpl<T, Index, T, Rest...> : std::integral_constant<std::size_t, Index> {};

template <typename T>
struct LinearBufferMinElementSize : std::integral_constant<std::size_t, sizeof(T)> {};

template <typename T>
struct LinearBufferMinElementSize<T[]> : std::integral_constant<std::size_t, sizeof(std::size_t)> {};

template <typename T>
struct LinearBufferVisitorParameterType {
	using type = const T;
};

template <typename T>
struct LinearBufferVisitorParameterType<T[]> {
	using type = std::span<const T>;
};

} // namespace detail

/// \cond
template <typename T, typename B>
struct linear_buffer_has_alternative;

template <typename T, typename First, typename... Rest>
struct linear_buffer_has_alternative<T, LinearBuffer<First, Rest...>> : linear_buffer_has_alternative<T, LinearBuffer<Rest...>> {};

template <typename T>
struct linear_buffer_has_alternative<T, LinearBuffer<>> : std::false_type {};

template <typename T, typename... Rest>
struct linear_buffer_has_alternative<T, LinearBuffer<T, Rest...>> : std::true_type {};
/// \endcond

template <typename T, typename B>
inline constexpr bool linear_buffer_has_alternative_v = linear_buffer_has_alternative<T, B>::value;

/// \cond
template <typename T, typename B>
struct linear_buffer_index;

template <typename T, typename... Ts>
struct linear_buffer_index<T, LinearBuffer<Ts...>> : detail::LinearBufferIndexImpl<T, 0, Ts...> {};
/// \endcond

template <typename T, typename B>
inline constexpr std::size_t linear_buffer_index_v = linear_buffer_index<T, B>::value;

/// \cond
template <std::size_t Index, typename B>
struct linear_buffer_alternative;

template <std::size_t Index, typename First, typename... Rest>
struct linear_buffer_alternative<Index, LinearBuffer<First, Rest...>> : linear_buffer_alternative<Index - 1, LinearBuffer<Rest...>> {};

template <typename T, typename... Rest>
struct linear_buffer_alternative<0, LinearBuffer<T, Rest...>> {
	using type = T;
};
/// \endcond

template <std::size_t Index, typename B>
using linear_buffer_alternative_t = typename linear_buffer_alternative<Index, B>::type;

/// \cond
template <typename B>
struct linear_buffer_size;

template <typename... Ts>
struct linear_buffer_size<LinearBuffer<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)> {};
/// \endcond

template <typename B>
inline constexpr std::size_t linear_buffer_size_v = linear_buffer_size<B>::value;

template <typename... Ts>
class LinearBuffer {
public:
	static_assert((std::is_trivially_copyable_v<std::remove_extent_t<Ts>> && ...), "LinearBuffer requires all element types to be trivially copyable.");

	// clang-format off
    using index_type =
        std::conditional_t<sizeof...(Ts) < 255ull, std::uint8_t,
        std::conditional_t<sizeof...(Ts) < 65535ull, std::uint16_t,
        std::conditional_t<sizeof...(Ts) < 4294967295ull, std::uint32_t,
        std::uint64_t>>>;
	// clang-format on

	static constexpr index_type npos = sizeof...(Ts);

	explicit LinearBuffer(LinearMemoryResource* memoryResource, std::size_t nextChunkSize = 64) noexcept
		: memoryResource(memoryResource)
		, nextChunkSize(std::max(nextChunkSize, MIN_CHUNK_SIZE)) {
		assert(memoryResource);
	}

	template <typename T>
	void push_back(const T& value) requires(!std::is_unbounded_array_v<T> && linear_buffer_has_alternative_v<T, LinearBuffer>) {
		constexpr std::size_t HEADER_SIZE = sizeof(index_type);
		constexpr std::size_t requiredSize = HEADER_SIZE + sizeof(T) + sizeof(index_type) + sizeof(std::byte*);
		const std::size_t remainingMemorySize = static_cast<std::size_t>(remainingMemoryEnd - remainingMemoryBegin);
		if (remainingMemorySize < requiredSize) {
			[[unlikely]];
			const std::size_t newChunkSize = std::max(requiredSize, nextChunkSize);
			if (head) {
				assert(remainingMemorySize >= sizeof(index_type) + sizeof(std::byte*));
				std::byte* const oldChunkTail = remainingMemoryBegin;
				std::byte* const newChunk = allocateChunk(newChunkSize);
				static_assert(sizeof(npos) == sizeof(index_type));
				std::memcpy(oldChunkTail, &npos, sizeof(index_type));
				std::memcpy(oldChunkTail + sizeof(index_type), &newChunk, sizeof(std::byte*));
			} else {
				head = allocateChunk(newChunkSize);
			}
		}
		constexpr index_type index = linear_buffer_index_v<T, LinearBuffer>;
		std::memcpy(remainingMemoryBegin, &index, sizeof(index_type));
		std::memcpy(remainingMemoryBegin + HEADER_SIZE, &value, sizeof(T));
		remainingMemoryBegin += HEADER_SIZE + sizeof(T);
	}

	template <typename T, typename... Args>
	void emplace_back(Args&&... args) requires(!std::is_unbounded_array_v<T> && linear_buffer_has_alternative_v<T, LinearBuffer> && std::is_constructible_v<T, Args...>) {
		return push_back<T>(T{std::forward<Args>(args)...});
	}

	template <typename T>
	std::span<const T> append(std::span<const T> values) requires(linear_buffer_has_alternative_v<T[], LinearBuffer>) {
		constexpr std::size_t HEADER_SIZE = sizeof(index_type) + sizeof(std::size_t);
		const std::size_t remainingMemorySize = static_cast<std::size_t>(remainingMemoryEnd - remainingMemoryBegin);
		void* alignedPointer = remainingMemoryBegin + HEADER_SIZE;
		std::size_t space = remainingMemorySize - HEADER_SIZE;
		if (remainingMemorySize < HEADER_SIZE || (!values.empty() && !std::align(alignof(T), values.size_bytes(), alignedPointer, space))) {
			[[unlikely]];
			const std::size_t requiredSize = HEADER_SIZE + alignof(T) - 1 + values.size_bytes() + sizeof(index_type) + sizeof(std::byte*);
			const std::size_t newChunkSize = std::max(requiredSize, nextChunkSize);
			if (head) {
				assert(remainingMemorySize >= sizeof(index_type) + sizeof(std::byte*));
				std::byte* const oldChunkTail = remainingMemoryBegin;
				std::byte* const newChunk = allocateChunk(newChunkSize);
				static_assert(sizeof(npos) == sizeof(index_type));
				std::memcpy(oldChunkTail, &npos, sizeof(index_type));
				std::memcpy(oldChunkTail + sizeof(index_type), &newChunk, sizeof(std::byte*));
			} else {
				head = allocateChunk(newChunkSize);
			}
			alignedPointer = remainingMemoryBegin + HEADER_SIZE;
			space = static_cast<std::size_t>(remainingMemoryEnd - remainingMemoryBegin) - HEADER_SIZE;
			[[maybe_unused]] void* const aligned = std::align(alignof(T), values.size_bytes(), alignedPointer, space);
			assert(aligned);
		}
		constexpr index_type index = linear_buffer_index_v<T[], LinearBuffer>;
		const std::size_t count = values.size();
		std::memcpy(remainingMemoryBegin, &index, sizeof(index_type));
		std::memcpy(remainingMemoryBegin + sizeof(index_type), &count, sizeof(std::size_t));
		std::memcpy(alignedPointer, values.data(), values.size_bytes());
		remainingMemoryBegin = static_cast<std::byte*>(alignedPointer) + values.size_bytes();
		return std::span<const T>{reinterpret_cast<const T*>(alignedPointer), count};
	}

	template <typename Visitor>
	auto visit(Visitor&& visitor) const {
		using R = std::common_type_t<decltype(std::invoke(std::forward<Visitor>(visitor), std::declval<typename detail::LinearBufferVisitorParameterType<Ts>::type>()))...>;
		const std::byte* const end = remainingMemoryBegin;
		for (const std::byte* pointer = head; pointer != end;) {
			index_type index;
			std::memcpy(&index, pointer, sizeof(index_type));
			pointer += sizeof(index_type);
			const auto apply = [&]<std::size_t Index>(std::in_place_index_t<Index>) -> void {
				using RawType = linear_buffer_alternative_t<Index, LinearBuffer>;
				if constexpr (std::is_unbounded_array_v<RawType>) {
					using T = std::remove_extent_t<RawType>;
					std::size_t count;
					std::memcpy(&count, pointer, sizeof(std::size_t));
					pointer += sizeof(std::size_t);
					if constexpr (alignof(T) > 1) {
						if (count > 0) {
							void* alignedPointer = const_cast<void*>(static_cast<const void*>(pointer));
							std::size_t remainingMemorySize = end - pointer;
							[[maybe_unused]] void* const aligned = std::align(alignof(T), count * sizeof(T), alignedPointer, remainingMemorySize);
							assert(aligned);
							pointer = static_cast<const std::byte*>(alignedPointer);
						}
					}
					const std::span<const T> values{reinterpret_cast<const T*>(pointer), count};
					if constexpr (std::is_void_v<R>) {
						std::invoke(std::forward<Visitor>(visitor), values);
						pointer += count * sizeof(T);
					} else {
						if (std::invoke(std::forward<Visitor>(visitor), values)) {
							pointer += count * sizeof(T);
						} else {
							pointer = end;
						}
					}
				} else {
					using T = RawType;
					alignas(T) std::array<std::byte, sizeof(T)> storage;
					std::memcpy(storage.data(), pointer, sizeof(T));
					const T& value = *std::launder(reinterpret_cast<const T*>(storage.data()));
					if constexpr (std::is_void_v<R>) {
						std::invoke(std::forward<Visitor>(visitor), value);
						pointer += sizeof(T);
					} else {
						if (std::invoke(std::forward<Visitor>(visitor), value)) {
							pointer += sizeof(T);
						} else {
							pointer = end;
						}
					}
				}
			};
			[&]<std::size_t... Indices>(std::index_sequence<Indices...>) -> void {
				if (!(((index == Indices) ? (apply(std::in_place_index<Indices>), true) : false) || ...)) {
					[[unlikely]];
					std::memcpy(&pointer, pointer, sizeof(std::byte*));
				}
			}(std::make_index_sequence<sizeof...(Ts)>{});
		}
		if constexpr (!std::is_void_v<R>) {
			return true;
		}
	}

private:
	static constexpr std::size_t MIN_CHUNK_SIZE = std::max({(sizeof(index_type) + detail::LinearBufferMinElementSize<Ts>::value + sizeof(index_type) + sizeof(std::byte*))...});

	[[nodiscard]] std::byte* allocateChunk(std::size_t newChunkSize) {
		assert(memoryResource);
		std::byte* const newChunk = static_cast<std::byte*>(memoryResource->allocate(newChunkSize, 1));
		remainingMemoryBegin = newChunk;
		remainingMemoryEnd = newChunk + newChunkSize;
		nextChunkSize += nextChunkSize / 2;
		return newChunk;
	}

	LinearMemoryResource* memoryResource;
	std::byte* head = nullptr;
	std::byte* remainingMemoryBegin = nullptr;
	std::byte* remainingMemoryEnd = nullptr;
	std::size_t nextChunkSize;
};

} // namespace donut

#endif
