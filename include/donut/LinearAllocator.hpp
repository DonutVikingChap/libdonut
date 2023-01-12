#ifndef DONUT_LINEAR_ALLOCATOR_HPP
#define DONUT_LINEAR_ALLOCATOR_HPP

#include <algorithm> // std::max
#include <cassert>   // assert
#include <cstddef>   // std::size_t, std::byte, std::max_align_t
#include <memory>    // std::align
#include <new>       // std::align_val_t
#include <span>      // std::span
#include <utility>   // std::exchange, std::swap
#include <vector>    // std::vector

namespace donut {

class LinearMemoryResource {
public:
	LinearMemoryResource() noexcept
		: LinearMemoryResource(std::span<std::byte>{}) {}

	explicit LinearMemoryResource(std::span<std::byte> initialMemory) noexcept
		: remainingMemoryBegin(initialMemory.data())
		, remainingMemorySize(initialMemory.size())
		, nextChunkSize(std::max(std::size_t{1024}, initialMemory.size() * GROWTH_FACTOR)) {}

	void* allocate(std::size_t size, std::size_t alignment) {
		if (size == 0) {
			[[unlikely]];
			size = 1;
		}
		void* result = std::align(alignment, size, remainingMemoryBegin, remainingMemorySize);
		if (!result) {
			[[unlikely]];
			const std::size_t newChunkSize = std::max(size, nextChunkSize);
			const std::size_t newChunkAlignment = std::max(alignment, alignof(std::max_align_t));
			AlignedHeapMemoryChunk& newChunk = extraMemory.emplace_back(newChunkSize, newChunkAlignment);
			remainingMemoryBegin = newChunk.memory;
			remainingMemorySize = newChunkSize;
			nextChunkSize *= GROWTH_FACTOR;
			result = remainingMemoryBegin;
		}
		remainingMemoryBegin = static_cast<std::byte*>(remainingMemoryBegin) + size;
		remainingMemorySize -= size;
		return result;
	}

private:
	struct AlignedHeapMemoryChunk {
		void* memory;
		std::size_t alignment;

		AlignedHeapMemoryChunk(std::size_t size, std::size_t alignment)
			: memory(operator new[](size, static_cast<std::align_val_t>(alignment)))
			, alignment(alignment) {}

		~AlignedHeapMemoryChunk() {
			operator delete[](memory, static_cast<std::align_val_t>(alignment));
		}

		AlignedHeapMemoryChunk(const AlignedHeapMemoryChunk&) = delete;

		AlignedHeapMemoryChunk(AlignedHeapMemoryChunk&& other) noexcept
			: memory(std::exchange(other.memory, nullptr))
			, alignment(std::exchange(other.alignment, 1)) {}

		AlignedHeapMemoryChunk& operator=(const AlignedHeapMemoryChunk&) = delete;

		AlignedHeapMemoryChunk& operator=(AlignedHeapMemoryChunk&& other) noexcept {
			std::swap(memory, other.memory);
			std::swap(alignment, other.alignment);
			return *this;
		}
	};

	static constexpr std::size_t GROWTH_FACTOR = 2;

	void* remainingMemoryBegin;
	std::size_t remainingMemorySize;
	std::size_t nextChunkSize;
	std::vector<AlignedHeapMemoryChunk> extraMemory{};
};

template <typename T>
class LinearAllocator {
public:
	using value_type = T;

	LinearAllocator(LinearMemoryResource* memoryResource) noexcept
		: memoryResource(memoryResource) {
		assert(memoryResource);
	}

	template <typename U>
	LinearAllocator(const LinearAllocator<U>& other) noexcept
		: memoryResource(other.memoryResource) {}

	LinearAllocator(const LinearAllocator& other) noexcept = default;
	LinearAllocator(LinearAllocator&& other) noexcept = default;
	LinearAllocator& operator=(const LinearAllocator& other) noexcept = default;
	LinearAllocator& operator=(LinearAllocator&& other) noexcept = default;

	[[nodiscard]] T* allocate(std::size_t n) {
		return static_cast<T*>(memoryResource->allocate(n * sizeof(T), alignof(T)));
	}

	void deallocate(T*, std::size_t) noexcept {}

	template <typename U>
	[[nodiscard]] bool operator==(const LinearAllocator<U>& other) const noexcept {
		return memoryResource == other.memoryResource;
	}

private:
	LinearMemoryResource* memoryResource;
};

} // namespace donut

#endif
