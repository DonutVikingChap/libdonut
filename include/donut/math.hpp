#ifndef DONUT_MATH_HPP
#define DONUT_MATH_HPP

#include <glm/ext/matrix_clip_space.hpp>    // glm::ortho, glm::perspective
#include <glm/ext/matrix_transform.hpp>     // glm::identity, glm::translate, glm::rotate, glm::scale, glm::lookAt
#include <glm/glm.hpp>                      // glm::...
#include <glm/gtc/matrix_inverse.hpp>       // glm::inverseTranspose
#include <glm/gtc/quaternion.hpp>           // glm::qua, glm::quat, glm::quat_cast, glm::lerp, glm::slerp
#include <glm/gtc/type_ptr.hpp>             // glm::value_ptr
#include <glm/gtx/euler_angles.hpp>         // glm::orientate2, glm::orientate3, glm::orientate4
#include <glm/gtx/matrix_interpolation.hpp> // glm::interpolate
#include <glm/gtx/norm.hpp>                 // glm::length2, glm::distance2
#include <glm/gtx/transform.hpp>            // glm::translate, glm::rotate, glm::scale
#include <numbers>                          // std::numbers::...

namespace donut {

namespace numbers {

using std::numbers::e_v;
using std::numbers::egamma_v;
using std::numbers::inv_pi_v;
using std::numbers::inv_sqrt3_v;
using std::numbers::inv_sqrtpi_v;
using std::numbers::ln10_v;
using std::numbers::ln2_v;
using std::numbers::log10e_v;
using std::numbers::log2e_v;
using std::numbers::phi_v;
using std::numbers::pi_v;
using std::numbers::sqrt2_v;
using std::numbers::sqrt3_v;

inline constexpr float e = e_v<float>;
inline constexpr float log2e = log2e_v<float>;
inline constexpr float log10e = log10e_v<float>;
inline constexpr float pi = pi_v<float>;
inline constexpr float inv_pi = inv_pi_v<float>;
inline constexpr float inv_sqrtpi = inv_sqrtpi_v<float>;
inline constexpr float ln2 = ln2_v<float>;
inline constexpr float ln10 = ln10_v<float>;
inline constexpr float sqrt2 = sqrt2_v<float>;
inline constexpr float sqrt3 = sqrt3_v<float>;
inline constexpr float inv_sqrt3 = inv_sqrt3_v<float>;
inline constexpr float egamma = egamma_v<float>;
inline constexpr float phi = phi_v<float>;

} // namespace numbers

using glm::abs;
using glm::acos;
using glm::asin;
using glm::atan;
using glm::atan2;
using glm::ceil;
using glm::clamp;
using glm::cos;
using glm::cross;
using glm::degrees;
using glm::distance2;
using glm::dmat2;
using glm::dmat3;
using glm::dmat4;
using glm::dot;
using glm::dvec2;
using glm::dvec3;
using glm::dvec4;
using glm::exp;
using glm::exp2;
using glm::floor;
using glm::fract;
using glm::i16;
using glm::i16vec2;
using glm::i16vec3;
using glm::i16vec4;
using glm::i32;
using glm::i32vec2;
using glm::i32vec3;
using glm::i32vec4;
using glm::i64;
using glm::i64vec2;
using glm::i64vec3;
using glm::i64vec4;
using glm::i8;
using glm::i8vec2;
using glm::i8vec3;
using glm::i8vec4;
using glm::identity;
using glm::interpolate;
using glm::inverse;
using glm::inverseTranspose;
using glm::ivec2;
using glm::ivec3;
using glm::ivec4;
using glm::length;
using glm::length2;
using glm::length_t;
using glm::lerp;
using glm::lookAt;
using glm::mat;
using glm::mat2;
using glm::mat3;
using glm::mat3_cast;
using glm::mat4;
using glm::mat4_cast;
using glm::max;
using glm::min;
using glm::mix;
using glm::normalize;
using glm::orientate2;
using glm::orientate3;
using glm::orientate4;
using glm::ortho;
using glm::perspective;
using glm::pow;
using glm::qua;
using glm::quat;
using glm::quat_cast;
using glm::radians;
using glm::rotate;
using glm::round;
using glm::scale;
using glm::sin;
using glm::slerp;
using glm::sqrt;
using glm::tan;
using glm::translate;
using glm::transpose;
using glm::u16;
using glm::u16vec2;
using glm::u16vec3;
using glm::u16vec4;
using glm::u32;
using glm::u32vec2;
using glm::u32vec3;
using glm::u32vec4;
using glm::u64;
using glm::u64vec2;
using glm::u64vec3;
using glm::u64vec4;
using glm::u8;
using glm::u8vec2;
using glm::u8vec3;
using glm::u8vec4;
using glm::uvec2;
using glm::uvec3;
using glm::uvec4;
using glm::value_ptr;
using glm::vec;
using glm::vec2;
using glm::vec3;
using glm::vec4;

} // namespace donut

#endif
