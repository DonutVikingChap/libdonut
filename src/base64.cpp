#include <donut/base64.hpp>

#include <array>       // std::array
#include <cstddef>     // std::size_t
#include <cstdint>     // std::uint32_t
#include <stdexcept>   // std::invalid_argument
#include <string>      // std::string
#include <string_view> // std::string_view

namespace donut::base64 {

namespace {

// clang-format off
constexpr std::array<char, 64> ENCODING_TABLE{ 
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/',
};
// clang-format on

// clang-format off
constexpr std::array<unsigned char, 256> DECODING_TABLE{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x3F,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
    0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
// clang-format on

} // namespace

std::string encode(std::string_view data) {
	constexpr std::array<std::size_t, 3> PADDING_MOD_TABLE{0, 2, 1};

	const std::size_t size = data.size();
	const std::size_t encodedSize = 4 * ((size + 2) / 3);

	std::string result{};
	result.reserve(encodedSize);

	for (std::size_t i = 0; i < size;) {
		const unsigned char octetA = static_cast<unsigned char>(data[i++]);
		const unsigned char octetB = i < size ? static_cast<unsigned char>(data[i++]) : 0;
		const unsigned char octetC = i < size ? static_cast<unsigned char>(data[i++]) : 0;
		const std::uint32_t triple = (static_cast<std::uint32_t>(octetA) << 16) | (static_cast<std::uint32_t>(octetB) << 8) | static_cast<std::uint32_t>(octetC);
		result.push_back(ENCODING_TABLE[(triple >> 3 * 6) & 0x3F]);
		result.push_back(ENCODING_TABLE[(triple >> 2 * 6) & 0x3F]);
		result.push_back(ENCODING_TABLE[(triple >> 1 * 6) & 0x3F]);
		result.push_back(ENCODING_TABLE[(triple >> 0 * 6) & 0x3F]);
	}

	const std::size_t padding = PADDING_MOD_TABLE[size % 3];
	for (std::size_t i = 0; i < padding; i++) {
		result[encodedSize - 1 - i] = '=';
	}

	return result;
}

std::string decode(std::string_view string) {
	const std::size_t size = string.size();
	if (size % 4 != 0) {
		throw std::invalid_argument{"Invalid base64 string length."};
	}

	const std::size_t padding = (size >= 4) ? ((string[size - 1] == '=') + (string[size - 2] == '=')) : 0;
	const std::size_t decodedSize = size / 4 * 3 - padding;

	std::string result{};
	result.reserve(decodedSize);

	for (std::size_t i = 0; i < size;) {
		const unsigned char sextetA = DECODING_TABLE[static_cast<unsigned char>(string[i++])];
		const unsigned char sextetB = DECODING_TABLE[static_cast<unsigned char>(string[i++])];
		const unsigned char sextetC = DECODING_TABLE[static_cast<unsigned char>(string[i++])];
		const unsigned char sextetD = DECODING_TABLE[static_cast<unsigned char>(string[i++])];
		const std::uint32_t triple =                         //
			(static_cast<std::uint32_t>(sextetA) << 3 * 6) + //
			(static_cast<std::uint32_t>(sextetB) << 2 * 6) + //
			(static_cast<std::uint32_t>(sextetC) << 1 * 6) + //
			(static_cast<std::uint32_t>(sextetD) << 0 * 6);
		if (result.size() < decodedSize) {
			result.push_back(static_cast<char>((triple >> 2 * 8) & 0xFF));
		}
		if (result.size() < decodedSize) {
			result.push_back(static_cast<char>((triple >> 1 * 8) & 0xFF));
		}
		if (result.size() < decodedSize) {
			result.push_back(static_cast<char>((triple >> 0 * 8) & 0xFF));
		}
	}

	return result;
}

} // namespace donut::base64
