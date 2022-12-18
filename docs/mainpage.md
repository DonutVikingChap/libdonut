# API Reference for libdonut

Libdonut is an application framework for cross-platform game development in C++20.

## Getting started

The source code for libdonut can be found on [GitHub](https://github.com/DonutVikingChap/libdonut). See the included `README.md` file for instructions on how to configure and build a new application project.

## Examples

Libdonut includes example code in the `examples/` directory that can be used as a reference for how various features of the library work and are intended to be used. All examples use the included `examples/data/` folder as their main resource directory for any associated assets that need to be loaded at runtime.

### Example game

The main example is a simple game project consisting of a single source file, `examples/example_game.cpp`, with the main application class at the beginning and the main program function at the bottom. This game can be used to study how various libdonut features are combined to form a working application. Note however that for a real project, the code that this example represents would typically be split across multiple files to make the main application file less cluttered.

## Main modules

The main API of libdonut is organized into the following modules:

- [donut::application](@ref donut::application) - Application framework
- [donut::audio](@ref donut::audio) - Audio engine
- [donut::graphics](@ref donut::graphics) - Graphics rendering

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
    - [donut::unicode](@ref donut::unicode) - UTF8 text decoding
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
