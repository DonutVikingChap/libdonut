#ifndef DONUT_ALIASES_HPP
#define DONUT_ALIASES_HPP

#include <donut/modules/fwd/application.hpp>
#include <donut/modules/fwd/audio.hpp>
#include <donut/modules/fwd/graphics.hpp>
#include <donut/modules/fwd/utilities.hpp>

#include <glm/fwd.hpp> // glm::length_t

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

template <std::size_t InitialResolution, std::size_t Padding>
using AtlasPacker = donut::AtlasPacker<InitialResolution, Padding>;

using Color = donut::Color;

using File = donut::File;

using InputFileStream = donut::InputFileStream;

using LinearMemoryResource = donut::LinearMemoryResource;
template <typename T>
using LinearAllocator = donut::LinearAllocator<T>;

using OutputFileStream = donut::OutputFileStream;

template <typename Handle, typename Deleter, Handle NullHandle>
using Resource = donut::Resource<Handle, Deleter, NullHandle>;

template <typename T>
using Circle = donut::Circle<T>;

template <glm::length_t L, typename T>
using AxisAlignedBox = donut::AxisAlignedBox<L, T>;

template <typename T>
using Rectangle = donut::Rectangle<T>;

template <typename T>
using Timer = donut::Timer<T>;

template <typename... Ts>
using Variant = donut::Variant<Ts...>;

#endif
