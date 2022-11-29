#ifndef DONUT_BASE64_HPP
#define DONUT_BASE64_HPP

#include <string>
#include <string_view>

namespace donut {

std::string base64Encode(std::string_view data);
std::string base64Decode(std::string_view data);

} // namespace donut

#endif
