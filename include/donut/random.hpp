#ifndef DONUT_RANDOM_HPP
#define DONUT_RANDOM_HPP

#include <algorithm> // std::generate
#include <array>     // std::array
#include <bit>       // std::rotl
#include <cstdint>   // std::uint64_t
#include <istream>   // std::basic_istream
#include <limits>    // std::numeric_limits
#include <ostream>   // std::basic_ostream

namespace donut::random {

/**
 * Implementation of the SplitMix64 pseudorandom number generator that provides
 * the API required for a standard uniform random bit generator, so that it can
 * be plugged into any of the random number distributions provided by the
 * standard library.
 *
 * This engine should typically only be used for seeding the
 * Xoroshiro128PlusPlusEngine, which should be preferred for general use.
 *
 * \warning This engine does not produce cryptographcially secure randomness and
 *          should not be used for such purposes.
 *
 * \sa Xoroshiro128PlusPlusEngine
 */
class SplitMix64Engine {
public:
	using result_type = std::uint64_t;

	static constexpr result_type default_seed = 0;

	[[nodiscard]] static constexpr result_type min() {
		return 0;
	}

	[[nodiscard]] static constexpr result_type max() {
		return std::numeric_limits<result_type>::max();
	}

	constexpr SplitMix64Engine() noexcept
		: SplitMix64Engine(default_seed) {}

	constexpr explicit SplitMix64Engine(result_type value) noexcept
		: state(value) {}

	constexpr void seed(result_type value = default_seed) noexcept {
		state = value;
	}

	constexpr result_type operator()() noexcept {
		state += 0x9E3779B97F4A7C15ull;
		std::uint64_t z = state;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}

	constexpr void discard(unsigned long long z) noexcept {
		while (z-- > 0) {
			(*this)();
		}
	}

	[[nodiscard]] constexpr bool operator==(const SplitMix64Engine& other) const noexcept {
		return state == other.state;
	}

	template <typename CharT, typename Traits>
	friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& stream, const SplitMix64Engine& engine) {
		return stream << engine.state;
	}

	template <typename CharT, typename Traits>
	friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& stream, SplitMix64Engine& engine) {
		return stream >> engine.state;
	}

private:
	std::uint64_t state;
};

/**
 * Implementation of the xoroshiro128++ pseudorandom number generator that
 * provides the API required for a standard uniform random bit generator, so
 * that it can be plugged into any of the random number distributions provided
 * by the standard library.
 *
 * This engine is small, fast and fairly high quality compared to most of the
 * pseudorandom number generators in the standard library.
 *
 * \warning This engine does not produce cryptographcially secure randomness and
 *          should not be used for such purposes.
 *
 * \sa https://prng.di.unimi.it/ for more information.
 */
class Xoroshiro128PlusPlusEngine {
public:
	using result_type = std::uint64_t;

	static constexpr result_type default_seed = SplitMix64Engine::default_seed;

	[[nodiscard]] static constexpr result_type min() {
		return 0;
	}

	[[nodiscard]] static constexpr result_type max() {
		return std::numeric_limits<result_type>::max();
	}

	constexpr Xoroshiro128PlusPlusEngine() noexcept
		: Xoroshiro128PlusPlusEngine(default_seed) {}

	constexpr explicit Xoroshiro128PlusPlusEngine(result_type value) noexcept {
		seed(value);
	}

	constexpr void seed(result_type value = default_seed) noexcept {
		SplitMix64Engine stateGenerator{value};
		std::generate(state.begin(), state.end(), stateGenerator);
	}

	constexpr result_type operator()() noexcept {
		const std::uint64_t s0 = state[0];
		std::uint64_t s1 = state[1];
		const std::uint64_t result = std::rotl(s0 + s1, 17) + s0;
		s1 ^= s0;
		state[0] = std::rotl(s0, 49) ^ s1 ^ (s1 << 21);
		state[1] = std::rotl(s1, 28);
		return result;
	}

	constexpr void discard(unsigned long long z) noexcept {
		while (z-- > 0) {
			(*this)();
		}
	}

	/**
	 * Advance the internal state 2^64 times.
	 */
	constexpr void jump() noexcept {
		std::uint64_t s0 = 0;
		std::uint64_t s1 = 0;
		for (const std::uint64_t c : {0x2BD7A6A6E99C2DDCull, 0x0992CCAF6A6FCA05ull}) {
			for (int b = 0; b < 64; ++b) {
				if ((c & (std::uint64_t{1} << b)) != 0) {
					s0 ^= state[0];
					s1 ^= state[1];
				}
				(*this)();
			}
		}
		state[0] = s0;
		state[1] = s1;
	}

	/**
	 * Advance the internal state 2^96 times.
	 */
	constexpr void longJump() noexcept {
		std::uint64_t s0 = 0;
		std::uint64_t s1 = 0;
		for (const std::uint64_t c : {0x360FD5F2CF8D5D99ull, 0x9C6E6877736C46E3ull}) {
			for (int b = 0; b < 64; ++b) {
				if ((c & (std::uint64_t{1} << b)) != 0) {
					s0 ^= state[0];
					s1 ^= state[1];
				}
				(*this)();
			}
		}
		state[0] = s0;
		state[1] = s1;
	}

	[[nodiscard]] constexpr bool operator==(const Xoroshiro128PlusPlusEngine& other) const noexcept {
		return state == other.state;
	}

	template <typename CharT, typename Traits>
	friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& stream, const Xoroshiro128PlusPlusEngine& engine) {
		return stream << engine.state[0] << ' ' << engine.state[1];
	}

	template <typename CharT, typename Traits>
	friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& stream, Xoroshiro128PlusPlusEngine& engine) {
		return stream >> engine.state[0] >> engine.state[1];
	}

private:
	std::array<std::uint64_t, 2> state{};
};

} // namespace donut::random

#endif
