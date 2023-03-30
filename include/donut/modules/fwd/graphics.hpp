#ifndef DONUT_MODULES_FWD_GRAPHICS_HPP
#define DONUT_MODULES_FWD_GRAPHICS_HPP

#include <cstddef> // std::size_t
#include <cstdint> // std::uint32_t

namespace donut {
namespace graphics {

class Buffer;

struct Error;

struct FontOptions;
class Font;

class Framebuffer;

using Handle = std::uint32_t;

enum class PixelFormat : std::uint32_t;
enum class PixelComponentType : std::uint32_t;
class ImageView;
struct ImageSavePNGOptions;
struct ImageSaveBMPOptions;
struct ImageSaveTGAOptions;
struct ImageSaveJPGOptions;
struct ImageSaveHDROptions;
struct ImageOptions;
class Image;

enum class MeshBufferUsage : std::uint32_t;
enum class MeshPrimitiveType : std::uint32_t;
enum class MeshIndexType : std::uint32_t;
struct NoIndex;
struct NoInstance;
template <typename Vertex, typename Index, typename Instance>
class Mesh;

struct Model;

struct RendererOptions;
class Renderer;

struct ModelInstance;
struct TextureInstance;
struct RectangleInstance;
struct QuadInstance;
struct SpriteInstance;
struct TextInstance;
class RenderPass;

struct ShaderConfiguration;

class ShaderParameter;
template <typename T, std::size_t N>
class ShaderArray;

struct ShaderProgramOptions;
class ShaderProgram;

enum class ShaderStageType : unsigned;
class ShaderStage;

struct Shader2DOptions;
class Shader2D;

struct Shader3DOptions;
class Shader3D;

class SpriteAtlas;

enum class TextureFormat : std::int32_t;
struct TextureOptions;
class Texture;

struct TexturedQuad;

class VertexArray;

struct Viewport;

using WindowId = std::uint32_t;
struct WindowOptions;
class Window;

} // namespace graphics
} // namespace donut

#endif
