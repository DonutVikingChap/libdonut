# API Reference for libdonut

Libdonut is an application framework for cross-platform game development in C++20.

## Getting started

The source code for libdonut can be found on [GitHub](https://github.com/DonutVikingChap/libdonut). See the included `README.md` file for instructions on how to configure and build a new application project.

## Examples

Libdonut includes example code in the `examples/` directory that can serve as a reference for how various features of the library work and are intended to be used. All examples use the included `examples/data/` folder as their main resource directory for any associated assets that need to be loaded at runtime. The main examples are:

- [example_game.cpp](@ref example_game.cpp) - Single-file game project showcasing various features
- [example_rectangle.cpp](@ref example_rectangle.cpp) - Very basic application that renders a rectangle

## Main modules

The main API of libdonut is organized into the following modules, listed along with a summary of their most important components:

- [donut::application](@ref donut::application) - Application framework module
    - [Application](@ref donut::application::Application) - Main application base class
- [donut::audio](@ref donut::audio) - Audio engine module
    - [Sound](@ref donut::audio::Sound) - Sound wave loading
    - [SoundStage](@ref donut::audio::SoundStage) - Sound playback in 3D to the default audio device
- [donut::events](@ref donut::events) - Events module
    - [EventPump](@ref donut::events::EventPump) - On-demand polling of events and user input from the host environment
    - [InputManager](@ref donut::events::InputManager) - Mapping between physical inputs and abstract output numbers
- [donut::graphics](@ref donut::graphics) - Graphics rendering module
    - [Camera](@ref donut::graphics::Camera) - Perspective to render from
    - [Font](@ref donut::graphics::Font) - Font loading for text rendering
    - [Framebuffer](@ref donut::graphics::Framebuffer) - Render target on the GPU
    - [Image](@ref donut::graphics::Image) - Image loading/saving
    - [Renderer](@ref donut::graphics::Renderer) - Rendering onto a framebuffer
    - [RenderPass](@ref donut::graphics::RenderPass) - Graphics drawing queue for batch rendering
    - [Model](@ref donut::graphics::Model) - Loading of 3D models
    - [Shader2D](@ref donut::graphics::Shader2D) - Shader program for instanced 2D textured quads
    - [Shader3D](@ref donut::graphics::Shader3D) - Shader program for instanced 3D models
    - [SpriteAtlas](@ref donut::graphics::SpriteAtlas) - Packing of sprites into an expandable spritesheet
    - [Text](@ref donut::graphics::Text) - Text shaping facility
    - [Texture](@ref donut::graphics::Texture) - Texture data stored on the GPU
    - [Viewport](@ref donut::graphics::Viewport) - Rectangular region of a framebuffer
    - [Window](@ref donut::graphics::Window) - Graphical window that can be rendered to

## Utility modules

Libdonut also includes the following utility APIs which encompass all of the main modules:

- Virtual filesystem:
    - [donut::File](@ref donut::File) - Virtual file handle
    - [donut::Filesystem](@ref donut::Filesystem) - Virtual filesystem
- Data interchange formats:
    - [donut::base64](@ref donut::base64) - Base64 string encoding/decoding
    - [donut::json](@ref donut::json) - JSON parsing/writing/(de)serialization
    - [donut::obj](@ref donut::obj) - OBJ and MTL file parsing
    - [donut::unicode](@ref donut::unicode) - UTF-8 text decoding
    - [donut::xml](@ref donut::xml) - XML document parsing
- Basic geometric shapes:
    - [donut::Point](@ref donut::Point) - Generic N-dimensional point
    - [donut::Length](@ref donut::Length) - Generic N-dimensional length
    - [donut::LineSegment](@ref donut::LineSegment) - Generic line segment between two points
    - [donut::Sphere](@ref donut::Sphere) - Generic sphere shape with a center and radius
    - [donut::Circle](@ref donut::Circle) - Flat 2D circle shape with a center and radius
    - [donut::Capsule](@ref donut::Capsule) - Generic capsule shape with a center line segment and radius
    - [donut::Box](@ref donut::Box) - Generic AAB shape with min and max extents
    - [donut::Rectangle](@ref donut::Rectangle) - Flat 2D rectangle shape with a position and size
- Data types:
    - [donut::Color](@ref donut::Color) - Floating-point RGBA color type
    - [donut::UniqueHandle](@ref donut::UniqueHandle) - Generic resource handle with exclusive ownership
    - [donut::Variant](@ref donut::Variant) - Generic tagged union type
- Other:
    - [donut::AtlasPacker](@ref donut::AtlasPacker) - Rectangle packer for expandable square texture atlases
    - [donut::LooseQuadtree](@ref donut::LooseQuadtree) - Container for fast AABB collision tests between a large number of 2D objects
    - [donut::random](@ref donut::random) - Fast pseudo-random number generation
    - [donut::reflection](@ref donut::reflection) - Compile-time reflection of aggregate types
    - [donut::Time](@ref donut::Time) - Time duration wrapper

## Includes

For ease of use, libdonut provides the following header files which include collections of headers from each respective module:

```cpp
#include <donut/modules/application.hpp>
#include <donut/modules/audio.hpp>
#include <donut/modules/events.hpp>
#include <donut/modules/graphics.hpp>
#include <donut/modules/utilities.hpp>
```

Alternatively, all modules can be included as follows:

```cpp
#include <donut/donut.hpp>
```

### Aliases

In addition, for applications where the potential naming conflicts are not a problem, the following include can be used to provide short, global aliases for the libdonut API:

```cpp
#include <donut/aliases.hpp>
```

These aliases include global declarations for all types that are defined directly in the `donut` namespace, such as `Color` for `donut::Color`, as well as the following namespace aliases:

```cpp
namespace app = donut::application;
namespace audio = donut::audio;
namespace events = donut::events;
namespace gfx = donut::graphics;

namespace base64 = donut::base64;
namespace json = donut::json;
namespace numbers = donut::numbers;
namespace obj = donut::obj;
namespace rng = donut::random;
namespace reflection = donut::reflection;
namespace unicode = donut::unicode;
namespace xml = donut::xml;
```

## Conventions

Except where the documentation specifies otherwise, the API uses the following conventions by default:

- The documentation uses the word "must" to specify that failure to meet a condition results in undefined behavior.
- The "detail" namespace is used for private implementation details which should not be used directly.
- Functions provide the basic exception guarantee, meaning that objects are left in a valid but unspecified state with all invariants preserved and no resources leaked.
- Any function that is not marked noexcept may be extended to throw any exception type in a future version of the library.
- Pointers, views and references returned from a member function are tied to the lifetime of the object on which the function was called.
- Non-nullptr pointers, views and references passed to a function, either directly or as part of an object, must remain valid until the function returns.
- When multiple pointers, views or references are passed to a function, they may not alias each other, nor reference overlapping memory regions.
- Operations which mutate the state of an object are not thread-safe unless otherwise noted, and require exclusive access to the object until the operation has finished.
- Functions defined in the [donut::graphics](@ref donut::graphics) module may only be called inside the main thread of the program.
- Types defined in the [donut::graphics](@ref donut::graphics) module may only be instantiated, referenced and destroyed inside the main thread of the program.
- World coordinates and physical quantities are expressed in SI units.
- Graphics and linear algebra operations follow OpenGL conventions, such as normalized texture and clip space coordinates and a right-handed coordinate system with Y up.
