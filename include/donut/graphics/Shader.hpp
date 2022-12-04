#ifndef DONUT_GRAPHICS_SHADER_HPP
#define DONUT_GRAPHICS_SHADER_HPP

#include <donut/Resource.hpp>
#include <donut/graphics/Handle.hpp>

#include <array>       // std::array
#include <cstddef>     // std::size_t
#include <cstdint>     // std::int32_t
#include <string>      // std::string, std::to_string
#include <string_view> // std::string_view
#include <utility>     // std::index_sequence, std::make_index_sequence

namespace donut {
namespace graphics {

enum class ShaderStageType : unsigned {
	VERTEX_SHADER = 0x8B31,
	FRAGMENT_SHADER = 0x8B30,
};

class ShaderStage {
public:
	constexpr explicit ShaderStage() noexcept = default;

	explicit ShaderStage(ShaderStageType type, std::string_view sourceCode);

	explicit operator bool() const noexcept {
		return static_cast<bool>(shader);
	}

	[[nodiscard]] Handle get() const noexcept {
		return shader.get();
	}

private:
	struct ShaderDeleter {
		void operator()(Handle handle) const noexcept;
	};

	Resource<Handle, ShaderDeleter> shader{};
};

struct ShaderProgramOptions {
	std::string_view vertexShaderSourceCode{};
	std::string_view fragmentShaderSourceCode{};
};

class ShaderProgram {
public:
	constexpr explicit ShaderProgram() noexcept = default;

	explicit ShaderProgram(const ShaderProgramOptions& options);

	explicit operator bool() const noexcept {
		return static_cast<bool>(program);
	}

	[[nodiscard]] Handle get() const noexcept {
		return program.get();
	}

private:
	struct ProgramDeleter {
		void operator()(Handle handle) const noexcept;
	};

	Resource<Handle, ProgramDeleter> program{};
	ShaderStage vertexShader{};
	ShaderStage fragmentShader{};
};

class ShaderUniform {
public:
	ShaderUniform(const ShaderProgram& program, const char* name);

	[[nodiscard]] std::int32_t getLocation() const noexcept {
		return location;
	}

private:
	std::int32_t location;
};

template <typename T, std::size_t N>
class ShaderArray {
public:
	ShaderArray(const ShaderProgram& program, const char* name)
		: array([&]<std::size_t... Indices>(std::index_sequence<Indices...>)->std::array<T, N> {
			return {(T{program, (std::string{name} + "[" + std::to_string(Indices) + "]").c_str()})...};
		}(std::make_index_sequence<N>{})) {}

	[[nodiscard]] std::size_t size() const noexcept {
		return array.size();
	}

	[[nodiscard]] T& operator[](std::size_t i) {
		return array[i];
	}

	[[nodiscard]] const T& operator[](std::size_t i) const {
		return array[i];
	}

	[[nodiscard]] decltype(auto) begin() const noexcept {
		return array.begin();
	}

	[[nodiscard]] decltype(auto) end() const noexcept {
		return array.end();
	}

private:
	std::array<T, N> array;
};

} // namespace graphics
} // namespace donut

#endif
