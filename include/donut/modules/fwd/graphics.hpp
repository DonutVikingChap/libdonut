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

class ImageView;
struct ImageOptions;
class Image;

class ImageHDRView;
struct ImageHDRSaveHDROptions;
struct ImageHDROptions;
class ImageHDR;

class ImageLDRView;
struct ImageLDRSavePNGOptions;
struct ImageLDRSaveBMPOptions;
struct ImageLDRSaveTGAOptions;
struct ImageLDRSaveJPGOptions;
struct ImageLDROptions;
class ImageLDR;

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

struct ShaderProgramOptions;
class ShaderProgram;

enum class ShaderStageType : unsigned;
class ShaderStage;

class ShaderUniform;
template <typename T, std::size_t N>
class ShaderArray;

struct Shader2DOptions;
class Shader2D;

struct Shader3DOptions;
class Shader3D;

class SpriteAtlas;

enum class TextureFormat : std::uint32_t;
enum class TextureInternalFormat : std::int32_t;
enum class TextureComponentType : std::uint32_t;
struct TextureOptions;
class Texture;

struct TexturedQuad;

class VertexArray;

struct Viewport;

} // namespace graphics
} // namespace donut

#endif
