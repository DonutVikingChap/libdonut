#ifndef DONUT_BASE64_HPP
#define DONUT_BASE64_HPP

#include <string>
#include <string_view>

namespace donut {
namespace base64 {

std::string encode(std::string_view data);
std::string decode(std::string_view data);

} // namespace base64
} // namespace donut

#endif
