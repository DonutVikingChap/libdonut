#ifndef DONUT_GRAPHICS_SHADER_PARAMETER_HPP
#define DONUT_GRAPHICS_SHADER_PARAMETER_HPP

#include <array>   // std::array
#include <cstddef> // std::size_t
#include <cstdint> // std::int32_t
#include <string>  // std::string, std::to_string
#include <utility> // std::index_sequence, std::make_index_sequence

namespace donut::graphics {

class ShaderProgram; // Forward declaration, to avoid a circular include of ShaderProgram.hpp.

/**
 * Identifier for a uniform shader variable inside a ShaderProgram.
 */
class ShaderParameter {
public:
	/**
	 * Construct an identifier for a specific uniform shader variable.
	 *
	 * \param program shader program in which the variable resides.
	 * \param name name of the variable.
	 *
	 * \note If the variable is not found, the resulting identifier will be
	 *       invalid.
	 */
	ShaderParameter(const ShaderProgram& program, const char* name);

	/**
	 * Get the location of the variable in the shader program.
	 *
	 * \return The location of the variable, or -1 if the identifier is invalid.
	 */
	[[nodiscard]] std::int32_t getLocation() const noexcept {
		return location;
	}

private:
	std::int32_t location;
};

/**
 * Fixed-size array of uniform shader variable identifiers representing an array
 * inside a ShaderProgram.
 *
 * \tparam T the type of uniform shader variable identifier to use for each
 *         element of the array.
 * \tparam N the number of elements in the array.
 */
template <typename T, std::size_t N>
class ShaderArray {
public:
	/**
	 * Construct an array of uniform shader variable identifiers for a specific
	 * shader array.
	 *
	 * \param program shader program in which the array resides.
	 * \param name name of the array.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \note If the array elements are not found, the resulting identifiers will
	 *       be invalid.
	 */
	ShaderArray(const ShaderProgram& program, const char* name)
		: array([&]<std::size_t... Indices>(std::index_sequence<Indices...>) -> std::array<T, N> {
			return {(T{program, (std::string{name} + "[" + std::to_string(Indices) + "]").c_str()})...};
		}(std::make_index_sequence<N>{})) {}

	/**
	 * Get the size of the array.
	 *
	 * \return the number of elements in the array, which is always equal to
	 *         template parameter N.
	 */
	[[nodiscard]] std::size_t size() const noexcept {
		return array.size();
	}

	/**
	 * Access an element of the array.
	 *
	 * \param i the index of the element to get. Must be less than the size of
	 *        the array.
	 *
	 * \return a reference to the element at index i.
	 */
	[[nodiscard]] T& operator[](std::size_t i) {
		return array[i];
	}

	/**
	 * Access an element of the array.
	 *
	 * \param i the index of the element to get. Must be less than the size of
	 *        the array.
	 *
	 * \return a read-only reference to the element at index i.
	 */
	[[nodiscard]] const T& operator[](std::size_t i) const {
		return array[i];
	}

	/**
	 * Get an iterator to the beginning of the array.
	 *
	 * \return the begin iterator.
	 */
	[[nodiscard]] decltype(auto) begin() const noexcept {
		return array.begin();
	}

	/**
	 * Get an iterator to the end of the array.
	 *
	 * \return the end iterator.
	 */
	[[nodiscard]] decltype(auto) end() const noexcept {
		return array.end();
	}

private:
	std::array<T, N> array;
};

} // namespace donut::graphics

#endif
