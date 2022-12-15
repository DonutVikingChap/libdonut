# libdonut

Libdonut is an application framework for cross-platform game development in C++20.

Note: This library is currently a **work in progress** and makes no guarantees about future version compatibility.

## Features

- Application framework:
    - [Application](include/donut/application/Application.hpp) base class:
        - Handles context setup and window management using [SDL](https://www.libsdl.org/).
        - Controls the main game loop, including [Event](include/donut/application/Event.hpp) pumping, frame pacing and fixed-interval framerate-independent tick updates.
        - Supports both native Windows/Linux compilation as well as the [emscripten](https://emscripten.org/) WebAssembly runtime.
    - [InputManager](include/donut/application/InputManager.hpp):
        - Maps physical [Input](include/donut/application/Input.hpp) events to abstract action numbers.
        - Supports simultaneous keyboard, mouse and controller input.
- Audio engine using [SoLoud](http://solhsa.com/soloud/):
    - [Sound](include/donut/audio/Sound.hpp) loading:
        - Support for OGG, WAV, FLAC and MP3 formats.
    - [SoundStage](include/donut/audio/SoundStage.hpp):
        - Plays 3D positional audio, background sounds and music.
        - Supports stopping, pausing, resuming, seeking and looping of playing sounds.
- Graphics rendering through [OpenGL](https://www.khronos.org/opengl/):
    - [RenderPass](include/donut/graphics/RenderPass.hpp) interface for simple batch rendering:
        - 3D Model rendering with built-in Blinn-Phong lighting or custom shaders through [Shader3D](include/donut/graphics/Shader3D.hpp).
        - 2D Textured quad rendering with built-in or custom shaders through [Shader2D](include/donut/graphics/Shader2D.hpp).
        - Sprite rendering with automatic [SpriteAtlas](include/donut/graphics/SpriteAtlas.hpp) packing.
        - Text rendering and [Font](include/donut/graphics/Font.hpp) loading using [libschrift](https://github.com/tomolt/libschrift).
    - [Renderer](include/donut/graphics/Renderer.hpp) supporting arbitrary [Framebuffer](include/donut/graphics/Framebuffer.hpp) targets and [Viewport](include/donut/graphics/Viewport.hpp) areas with optional integer scaling.
    - [Scene](include/donut/graphics/Scene.hpp) loading from OBJ files.
    - [Image](include/donut/graphics/Image.hpp) loading/saving in both 8-bit [LDR](include/donut/graphics/ImageLDR.hpp) and floating-point [HDR](include/donut/graphics/ImageHDR.hpp) using [stbi](https://github.com/nothings/stb).
- Utilities:
    - Floating-point RGBA [Color](include/donut/Color.hpp) type that includes predefined constants for common web colors.
    - Virtual [File](include/donut/File.hpp) system with streamed [reading](include/donut/InputFileStream.hpp) and [writing](include/donut/OutputFileStream.hpp) using [PhysicsFS](https://icculus.org/physfs/):
        - Supports automatic mounting of pak/zip/etc. archives at startup for easy mod loading.
        - Used for all built-in resource loading in libdonut.
    - Custom [Variant](include/donut/Variant.hpp) implementation:
        - Provides a cleaner API than std::variant by adding the methods `is<T>()`, `as<T>()`, `get<T>()` and `get_if<T>()` as well as a freestanding `match` function.
        - Visitation is implemented through chaining of the conditional operator rather than virtual method dispatch, making it easier for compilers to generate a jump table than in most common std::variant implementations. This puts `match` on par with a raw switch statement in terms of performance.
        - Supports being used as a base class in user code.
    - [Timer](include/donut/Timer.hpp) utility for counters that should trigger at a constant average interval, independent of the rate at which they are updated.
    - [Base64](include/donut/base64.hpp) encoding and decoding.
    - [JSON](include/donut/json.hpp) utilities:
        - Writing/parsing to/from strings and iostreams.
        - Extensible serialization/deserialization of arbitrary types.
        - Supports [JSON5](https://json5.org/) features (comments, trailing commas, identifier keys, etc.).
    - [OBJ](include/donut/obj.hpp) model/material parsing.
    - Fast [pseudo-random number generation engine](include/donut/random.hpp) using [Xoroshiro128++](https://prng.di.unimi.it/):
        - Designed to be used as a URBG in conjunction with the [standard C++ distributions](https://en.cppreference.com/w/cpp/numeric/random#Random_number_distributions).
    - [Compile-time reflection](include/donut/reflection.hpp) of aggregate types through some dark template magic.
        - Supports iterating references to the fields of an instance of any plain struct, with full type information available.
        - Used in libdonut to automate JSON serialization, OpenGL vertex attribute setup, etc.
    - Basic [shapes](include/donut/shapes.hpp) with intersection tests:
        - Circle
        - AxisAlignedBox
        - Rectangle
    - Utilities for working with [UTF8](include/donut/unicode.hpp) text:
        - Supports iterating unicode code points in any arbitrary byte sequence with basic error handling in the case of invalid encoding.
    - [XML](include/donut/xml.hpp) document parsing.

## Prerequisites

- CMake 3.15+
- gcc 12+/clang 14+/MSVC 17+

## Usage

```
TODO
```

## Authors

- Donut the Vikingchap: https://steamcommunity.com/id/donutvikingchap/

## License

Libdonut is distributed under the **MIT License**. See the included LICENSE file for more information.

### External libraries

Note that applications using libdonut must also respect the licenses of each of the following third-party libraries on which libdonut depends, as well as their respective dependencies:

- [{fmt}](https://github.com/fmtlib/fmt) (MIT License)
- [glad](https://github.com/Dav1dde/glad) (glad/gl.h is Public Domain/WTFPL/CC0, khrplatform.h is under MIT License)
- [GLM](https://github.com/g-truc/glm) (MIT License)
- [libschrift](https://github.com/tomolt/libschrift) (ISC License)
- [PhysicsFS](https://github.com/icculus/physfs) (zlib License)
- [SDL](https://github.com/libsdl-org/SDL) (zlib License)
- [SoLoud](https://github.com/jarikomppa/soloud) (zlib License)
- [stb](https://github.com/nothings/stb) (Public Domain)
