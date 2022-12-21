#ifndef DONUT_GRAPHICS_SHADER_HPP
#define DONUT_GRAPHICS_SHADER_HPP

#include <donut/Resource.hpp>
#include <donut/Variant.hpp>
#include <donut/graphics/Handle.hpp>

#include <array>       // std::array
#include <cstddef>     // std::size_t
#include <cstdint>     // std::int32_t
#include <glm/glm.hpp> // glm::...
#include <span>        // std::span
#include <string>      // std::string, std::to_string
#include <utility>     // std::pair, std::index_sequence, std::make_index_sequence
#include <vector>      // std::vector

namespace donut {
namespace graphics {

class ShaderUniform;

enum class ShaderStageType : unsigned {
	VERTEX_SHADER = 0x8B31,
	FRAGMENT_SHADER = 0x8B30,
};

class ShaderStage {
public:
	constexpr explicit ShaderStage() noexcept = default;

	explicit ShaderStage(ShaderStageType type, const char* definitions, const char* sourceCode);

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
	const char* definitions = nullptr;
	const char* vertexShaderSourceCode = nullptr;
	const char* fragmentShaderSourceCode = nullptr;
};

class ShaderProgram {
public:
	using UniformValue = Variant< //
		float,                    //
		glm::vec2,                //
		glm::vec3,                //
		glm::vec4,                //
		glm::i32,                 //
		glm::i32vec2,             //
		glm::i32vec3,             //
		glm::i32vec4,             //
		glm::u32,                 //
		glm::u32vec2,             //
		glm::u32vec3,             //
		glm::u32vec4,             //
		glm::mat2,                //
		glm::mat3,                //
		glm::mat4>;

	constexpr explicit ShaderProgram() noexcept = default;

	explicit ShaderProgram(const ShaderProgramOptions& options);

	explicit operator bool() const noexcept {
		return static_cast<bool>(program);
	}

	void setUniformFloat(const ShaderUniform& uniform, float value);
	void setUniformVec2(const ShaderUniform& uniform, glm::vec2 value);
	void setUniformVec3(const ShaderUniform& uniform, glm::vec3 value);
	void setUniformVec4(const ShaderUniform& uniform, glm::vec4 value);
	void setUniformInt(const ShaderUniform& uniform, glm::i32 value);
	void setUniformIVec2(const ShaderUniform& uniform, glm::i32vec2 value);
	void setUniformIVec3(const ShaderUniform& uniform, glm::i32vec3 value);
	void setUniformIVec4(const ShaderUniform& uniform, glm::i32vec4 value);
	void setUniformUint(const ShaderUniform& uniform, glm::u32 value);
	void setUniformUVec2(const ShaderUniform& uniform, glm::u32vec2 value);
	void setUniformUVec3(const ShaderUniform& uniform, glm::u32vec3 value);
	void setUniformUVec4(const ShaderUniform& uniform, glm::u32vec4 value);
	void setUniformMat2(const ShaderUniform& uniform, const glm::mat2& value);
	void setUniformMat3(const ShaderUniform& uniform, const glm::mat3& value);
	void setUniformMat4(const ShaderUniform& uniform, const glm::mat4& value);

	void clearUniformUploadQueue() noexcept {
		uniformUploadQueue.clear();
	}

	[[nodiscard]] std::span<const std::pair<std::int32_t, UniformValue>> getUniformUploadQueue() const noexcept {
		return uniformUploadQueue;
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
	std::vector<std::pair<std::int32_t, UniformValue>> uniformUploadQueue{};
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
