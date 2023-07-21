#ifndef DONUT_ALIASES_HPP
#define DONUT_ALIASES_HPP

#include <donut/modules/fwd/application.hpp>
#include <donut/modules/fwd/audio.hpp>
#include <donut/modules/fwd/graphics.hpp>
#include <donut/modules/fwd/utilities.hpp>

#include <glm/fwd.hpp> // glm::...

namespace app = donut::application;       // NOLINT(misc-unused-alias-decls)
namespace audio = donut::audio;           // NOLINT(misc-unused-alias-decls)
namespace gfx = donut::graphics;          // NOLINT(misc-unused-alias-decls)
namespace base64 = donut::base64;         // NOLINT(misc-unused-alias-decls)
namespace json = donut::json;             // NOLINT(misc-unused-alias-decls)
namespace obj = donut::obj;               // NOLINT(misc-unused-alias-decls)
namespace rng = donut::random;            // NOLINT(misc-unused-alias-decls)
namespace reflection = donut::reflection; // NOLINT(misc-unused-alias-decls)
namespace unicode = donut::unicode;       // NOLINT(misc-unused-alias-decls)
namespace xml = donut::xml;               // NOLINT(misc-unused-alias-decls)

using donut::AtlasPacker; // NOLINT(misc-unused-using-decls)

using donut::Color; // NOLINT(misc-unused-using-decls)

using donut::File; // NOLINT(misc-unused-using-decls)

using donut::InputFileStream; // NOLINT(misc-unused-using-decls)

using donut::LinearMemoryResource; // NOLINT(misc-unused-using-decls)

using donut::LinearAllocator; // NOLINT(misc-unused-using-decls)

using donut::LooseQuadtree; // NOLINT(misc-unused-using-decls)

using donut::OutputFileStream; // NOLINT(misc-unused-using-decls)

using donut::Resource; // NOLINT(misc-unused-using-decls)

template <glm::length_t L, typename T>
using Point = glm::vec<L, T>;

template <glm::length_t L, typename T>
using Length = glm::vec<L, T>;

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
