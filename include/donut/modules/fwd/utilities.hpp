#ifndef DONUT_MODULES_FWD_UTILITIES_HPP
#define DONUT_MODULES_FWD_UTILITIES_HPP

#include <cstddef>     // std::size_t
#include <cstdint>     // std::uint8_t
#include <glm/fwd.hpp> // glm::...

namespace donut {

template <std::size_t InitialResolution, std::size_t Padding>
class AtlasPacker;

namespace base64 {}

class Color;

class File;

enum class FilesystemMountPriority : int;
struct FilesystemOptions;
class Filesystem;

struct Monostate;

namespace json {

struct SourceLocation;
struct Error;
using Null = Monostate;
using Boolean = bool;
using Number = double;
class Object;
class Array;
class Value;
template <typename T>
struct Serializer;
template <typename T>
struct Deserializer;
struct SerializationOptions;
struct DeserializationOptions;
struct SerializationState;
struct DeserializationState;

} // namespace json

class LinearMemoryResource;
template <typename T>
class LinearAllocator;

template <typename T>
class LooseQuadtree;

namespace numbers {} // namespace numbers

using glm::dmat2;
using glm::dmat3;
using glm::dmat4;
using glm::dvec2;
using glm::dvec3;
using glm::dvec4;
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
using glm::ivec2;
using glm::ivec3;
using glm::ivec4;
using glm::length_t;
using glm::mat;
using glm::mat2;
using glm::mat3;
using glm::mat4;
using glm::qua;
using glm::quat;
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
using glm::vec;
using glm::vec2;
using glm::vec3;
using glm::vec4;

namespace obj {

struct Error;
struct FaceVertex;
struct Face;
struct Group;
struct Object;
struct Scene;

namespace mtl {

enum class IlluminationModel : std::uint8_t;
struct Material;
struct Library;

} // namespace mtl
} // namespace obj

namespace random {

class SplitMix64Engine;
class Xoroshiro128PlusPlusEngine;

} // namespace random

namespace reflection {}

template <length_t L, typename T>
struct LineSegment;

template <length_t L, typename T>
struct Sphere;

template <typename T>
struct Circle;

template <length_t L, typename T>
struct Capsule;

template <length_t L, typename T>
struct Box;

template <typename T>
struct Rectangle;

template <typename T, typename Period>
class Time;

namespace unicode {

struct EncodeUTF8FromCodePointResult;
struct UTF8Sentinel;
template <typename It, typename Sentinel>
class UTF8Iterator;
class UTF8View;

} // namespace unicode

template <typename Handle, typename Deleter, Handle NullHandle>
class UniqueHandle;

struct BadVariantAccess;
template <typename... Ts>
class Variant;

namespace xml {

struct Error;
struct Attribute;
struct Element;
struct Document;

} // namespace xml

} // namespace donut

#endif
