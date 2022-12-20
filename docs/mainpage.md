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
    - [InputManager](@ref donut::application::InputManager) - Mapping between physical inputs and abstract output numbers
- [donut::audio](@ref donut::audio) - Audio engine module
    - [Sound](@ref donut::audio::Sound) - Sound wave loading
    - [SoundStage](@ref donut::audio::SoundStage) - Sound playback in 3D to the default audio device
- [donut::graphics](@ref donut::graphics) - Graphics rendering module
    - [Font](@ref donut::graphics::Font) - Text shaping facility
    - [Framebuffer](@ref donut::graphics::Framebuffer) - Render target on the GPU
    - [ImageHDR](@ref donut::graphics::ImageHDR) - High dynamic range image loading/saving
    - [ImageLDR](@ref donut::graphics::ImageLDR) - Low dynamic range image loading/saving
    - [Renderer](@ref donut::graphics::Renderer) - Rendering onto a framebuffer
    - [RenderPass](@ref donut::graphics::RenderPass) - Graphics drawing queue for batch rendering
    - [Scene](@ref donut::graphics::Scene) - Scene loading of 3D models
    - [Shader2D](@ref donut::graphics::Shader2D) - Shader program for instanced 2D textured quads
    - [Shader3D](@ref donut::graphics::Shader3D) - Shader program for instanced 3D models
    - [SpriteAtlas](@ref donut::graphics::SpriteAtlas) - Packing of sprites into an expandable spritesheet
    - [Texture](@ref donut::graphics::Texture) - Texture data stored on the GPU
    - [Viewport](@ref donut::graphics::Viewport) - Rectangular region of a framebuffer

## Utility modules

Libdonut also includes the following utility APIs which encompass all of the main modules:

- Virtual filesystem:
    - [donut::File](@ref donut::File) - Abstract interface to virtual files
    - [donut::InputFileStream](@ref donut::InputFileStream) - Virtual file reading
    - [donut::OutputFileStream](@ref donut::OutputFileStream) - Virtual file writing
- Data interchange formats:
    - [donut::base64](@ref donut::base64) - Base64 string encoding/decoding
    - [donut::json](@ref donut::json) - JSON parsing/writing/(de)serialization
    - [donut::obj](@ref donut::obj) - OBJ and MTL file parsing
    - [donut::unicode](@ref donut::unicode) - UTF-8 text decoding
    - [donut::xml](@ref donut::xml) - XML document parsing
- Basic geometric shapes:
    - [donut::Circle](@ref donut::Circle) - Generic circle shape with a center and radius
    - [donut::AxisAlignedBox](@ref donut::AxisAlignedBox) - Generic AAB shape with min and max extents
    - [donut::Rectangle](@ref donut::Rectangle) - Generic rectangle shape with a position and size
- Data types:
    - [donut::Color](@ref donut::Color) - Floating-point RGBA color type
    - [donut::Resource](@ref donut::Resource) - Generic resource handle with exclusive ownership
    - [donut::Variant](@ref donut::Variant) - Generic tagged union type
- Other:
    - [donut::AtlasPacker](@ref donut::AtlasPacker) - Rectangle packer for expandable square texture atlases
    - [donut::random](@ref donut::random) - Fast pseudo-random number generation
    - [donut::reflection](@ref donut::reflection) - Compile-time reflection of aggregate types
    - [donut::Timer](@ref donut::Timer) - Time counting utility
