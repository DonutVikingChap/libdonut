#ifndef DONUT_ALIASES_HPP
#define DONUT_ALIASES_HPP

#include <donut/math.hpp>
#include <donut/modules/fwd/application.hpp>
#include <donut/modules/fwd/audio.hpp>
#include <donut/modules/fwd/events.hpp>
#include <donut/modules/fwd/graphics.hpp>
#include <donut/modules/fwd/utilities.hpp>

namespace app = donut::application;       // NOLINT(misc-unused-alias-decls)
namespace audio = donut::audio;           // NOLINT(misc-unused-alias-decls)
namespace events = donut::events;         // NOLINT(misc-unused-alias-decls)
namespace gfx = donut::graphics;          // NOLINT(misc-unused-alias-decls)
namespace base64 = donut::base64;         // NOLINT(misc-unused-alias-decls)
namespace json = donut::json;             // NOLINT(misc-unused-alias-decls)
namespace numbers = donut::numbers;       // NOLINT(misc-unused-alias-decls)
namespace obj = donut::obj;               // NOLINT(misc-unused-alias-decls)
namespace rng = donut::random;            // NOLINT(misc-unused-alias-decls)
namespace reflection = donut::reflection; // NOLINT(misc-unused-alias-decls)
namespace unicode = donut::unicode;       // NOLINT(misc-unused-alias-decls)
namespace xml = donut::xml;               // NOLINT(misc-unused-alias-decls)

using donut::AtlasPacker; // NOLINT(misc-unused-using-decls)

using donut::Color; // NOLINT(misc-unused-using-decls)

using donut::File; // NOLINT(misc-unused-using-decls)

using donut::FilesystemMountPriority; // NOLINT(misc-unused-using-decls)

using donut::FilesystemOptions; // NOLINT(misc-unused-using-decls)

using donut::Filesystem; // NOLINT(misc-unused-using-decls)

using donut::LinearMemoryResource; // NOLINT(misc-unused-using-decls)

using donut::LinearAllocator; // NOLINT(misc-unused-using-decls)

using donut::LooseQuadtree; // NOLINT(misc-unused-using-decls)

using donut::abs;
using donut::acos;
using donut::asin;
using donut::atan;
using donut::atan2;
using donut::ceil;
using donut::clamp;
using donut::cos;
using donut::cross;
using donut::degrees;
using donut::distance2;
using donut::dmat2;
using donut::dmat3;
using donut::dmat4;
using donut::dot;
using donut::dvec2;
using donut::dvec3;
using donut::dvec4;
using donut::exp;
using donut::exp2;
using donut::floor;
using donut::fract;
using donut::i16;
using donut::i16vec2;
using donut::i16vec3;
using donut::i16vec4;
using donut::i32;
using donut::i32vec2;
using donut::i32vec3;
using donut::i32vec4;
using donut::i64;
using donut::i64vec2;
using donut::i64vec3;
using donut::i64vec4;
using donut::i8;
using donut::i8vec2;
using donut::i8vec3;
using donut::i8vec4;
using donut::identity;
using donut::interpolate;
using donut::inverse;
using donut::inverseTranspose;
using donut::ivec2;
using donut::ivec3;
using donut::ivec4;
using donut::length;
using donut::length2;
using donut::length_t;
using donut::lerp;
using donut::lookAt;
using donut::mat;
using donut::mat2;
using donut::mat3;
using donut::mat3_cast;
using donut::mat4;
using donut::mat4_cast;
using donut::max;
using donut::min;
using donut::mix;
using donut::normalize;
using donut::orientate2;
using donut::orientate3;
using donut::orientate4;
using donut::ortho;
using donut::perspective;
using donut::pow;
using donut::qua;
using donut::quat;
using donut::quat_cast;
using donut::radians;
using donut::rotate;
using donut::round;
using donut::scale;
using donut::sin;
using donut::slerp;
using donut::sqrt;
using donut::tan;
using donut::translate;
using donut::transpose;
using donut::u16;
using donut::u16vec2;
using donut::u16vec3;
using donut::u16vec4;
using donut::u32;
using donut::u32vec2;
using donut::u32vec3;
using donut::u32vec4;
using donut::u64;
using donut::u64vec2;
using donut::u64vec3;
using donut::u64vec4;
using donut::u8;
using donut::u8vec2;
using donut::u8vec3;
using donut::u8vec4;
using donut::uvec2;
using donut::uvec3;
using donut::uvec4;
using donut::value_ptr;
using donut::vec;
using donut::vec2;
using donut::vec3;
using donut::vec4;

using donut::UniqueHandle; // NOLINT(misc-unused-using-decls)

template <donut::length_t L, typename T>
using Point = donut::vec<L, T>;

template <donut::length_t L, typename T>
using Length = donut::vec<L, T>;

using donut::LineSegment; // NOLINT(misc-unused-using-decls)

using donut::Sphere; // NOLINT(misc-unused-using-decls)

using donut::Circle; // NOLINT(misc-unused-using-decls)

using donut::Capsule; // NOLINT(misc-unused-using-decls)

using donut::Box; // NOLINT(misc-unused-using-decls)

using donut::Rectangle; // NOLINT(misc-unused-using-decls)

using donut::Time; // NOLINT(misc-unused-using-decls)

using donut::Monostate; // NOLINT(misc-unused-using-decls)

using donut::Variant; // NOLINT(misc-unused-using-decls)

#endif
