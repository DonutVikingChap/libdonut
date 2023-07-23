#ifndef DONUT_BASE64_HPP
#define DONUT_BASE64_HPP

#include <string>      // std::string
#include <string_view> // std::string_view

namespace donut::base64 {

/**
 * Encode arbitrary data into a Base64 string.
 *
 * \param data input data to encode.
 *
 * \return a Base64 string containing the encoded representation of the input.
 *
 * \throws std::length_error if the length of the encoded string exceeds the
 *         maximum supported string size.
 * \throws std::bad_alloc on allocation failure.
 */
std::string encode(std::string_view data);

/**
 * Decode the original data from a Base64 string.
 *
 * \param string Base64 string to decode.
 *
 * \return a byte sequence containing the data decoded from the Base64 string.
 *
 * \throws std::invalid_argument if the length of the Base64 string is not
 *         divisible by 4.
 * \throws std::length_error if the length of the decoded byte sequence exceeds
 *         the maximum supported string size.
 * \throws std::bad_alloc on allocation failure.
 */
std::string decode(std::string_view string);

} // namespace donut::base64

#endif
