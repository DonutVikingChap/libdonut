#ifndef DONUT_MODULES_FWD_UTILITIES_HPP
#define DONUT_MODULES_FWD_UTILITIES_HPP

#include <cstddef>     // std::size_t
#include <cstdint>     // std::uint8_t
#include <glm/fwd.hpp> // glm::length_t

namespace donut {

template <std::size_t InitialResolution, std::size_t Padding>
class AtlasPacker;

namespace base64 {}

class Color;

class File;

class InputFileStream;

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

class OutputFileStream;

namespace random {

class SplitMix64Engine;
class Xoroshiro128PlusPlusEngine;

} // namespace random

namespace reflection {}

template <typename Handle, typename Deleter, Handle NullHandle>
class Resource;

template <glm::length_t L, typename T>
struct LineSegment;

template <glm::length_t L, typename T>
struct Sphere;

template <typename T>
struct Circle;

template <glm::length_t L, typename T>
struct Capsule;

template <glm::length_t L, typename T>
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
