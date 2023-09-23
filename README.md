# libdonut

Libdonut is an application framework for cross-platform game development in C++20.

## Examples

- Template project with some instructional comments: [template/src/main.cpp](template/src/main.cpp)
- Single-file test game using various features: [examples/example_game.cpp](examples/example_game.cpp)
- Basic application that renders a rectangle: [examples/example_rectangle.cpp](examples/example_rectangle.cpp)

## Features

- Application framework:
    - [Application](include/donut/application/Application.hpp) base class:
        - Defines the main loop of the program in a platform-agnostic way that supports both native Windows/Linux compilation as well as the [emscripten](https://emscripten.org/) WebAssembly runtime.
        - Includes an optional built-in frame rate limiter.
        - Provides both per-frame update callbacks and frame rate-independent pacing of fixed-rate tick updates.
- Events system:
    - [EventPump](include/donut/events/EventPump.hpp):
        - Polls [Event](include/donut/events/Event.hpp) data from the environment on demand.
    - [InputManager](include/donut/events/InputManager.hpp):
        - Maps physical [Input](include/donut/events/Input.hpp) events to abstract action numbers.
        - Supports simultaneous keyboard, mouse, touch and controller input.
- Audio engine using [SoLoud](http://solhsa.com/soloud/):
    - [Sound](include/donut/audio/Sound.hpp) loading:
        - Supports OGG, WAV, FLAC and MP3 formats.
    - [SoundStage](include/donut/audio/SoundStage.hpp):
        - Plays 3D positional audio, background sounds and music.
- Graphics rendering through [OpenGL](https://www.khronos.org/opengl/):
    - [Window](include/donut/graphics/Window.hpp) abstraction:
        - Handles GL context setup and window management using [SDL](https://www.libsdl.org/).
    - [Renderer](include/donut/graphics/Renderer.hpp):
        - [RenderPass](include/donut/graphics/RenderPass.hpp) interface for simple batch rendering:
            - 3D Model rendering that supports custom shaders through [Shader3D](include/donut/graphics/Shader3D.hpp) or basic built-in Blinn-Phong lighting for prototyping.
            - 2D Textured quad rendering with built-in shaders or custom shaders through [Shader2D](include/donut/graphics/Shader2D.hpp).
            - Sprite rendering with automatic [SpriteAtlas](include/donut/graphics/SpriteAtlas.hpp) packing.
            - [Text](include/donut/graphics/Text.hpp) rendering and [Font](include/donut/graphics/Font.hpp) loading using [libschrift](https://github.com/tomolt/libschrift).
        - Supports arbitrary [Framebuffer](include/donut/graphics/Framebuffer.hpp) targets, [Camera](include/donut/graphics/Camera.hpp) positions and [Viewport](include/donut/graphics/Viewport.hpp) areas.
        - Viewports can be restricted to integer scaling for pixel-perfect fixed-resolution 2D rendering regardless of window size.
    - [Model](include/donut/graphics/Model.hpp) loading from OBJ files.
    - [Image](include/donut/graphics/Image.hpp) loading/saving using [stbi](https://github.com/nothings/stb).
- Utilities:
    - Hand-written parsers and writers for some common data formats:
        - [JSON](include/donut/json.hpp):
            - Serialize/deserialize arbitrary types to/from JSON.
            - Write/read JSON data to/from both strings and iostreams.
            - Supports memory-efficient visitor-based parsing.
            - Supports [JSON5](https://json5.org/) features (comments, trailing commas, identifier keys, etc.).
        - [OBJ](include/donut/obj.hpp):
            - Parse OBJ models and basic MTL materials.
        - [XML](include/donut/xml.hpp):
            - Parse XML documents into a simple tree structure in memory.
        - [Base64](include/donut/base64.hpp)
            - Encode/decode aribtrary data to/from standard Base64 strings.
        - [Unicode](include/donut/unicode.hpp):
            - Iterate the Unicode code points of UTF-8-encoded text in any sequence of bytes with simple error reporting in case of invalid encoding.
    - [AtlasPacker](include/donut/AtlasPacker.hpp) for packing rectangles into expandable square texture atlases.
    - Floating-point RGBA [Color](include/donut/Color.hpp) type that includes predefined constants for common web colors.
    - Virtual [Filesystem](include/donut/Filesystem.hpp) based on [PhysicsFS](https://icculus.org/physfs/):
        - Use virtual filepaths for uniform access to any resource [File](include/donut/File.hpp)  that resides in a mounted directory, regardless of its actual location.
        - Supports automatic mounting of pk3/zip/etc. archives at startup for easy mod loading.
        - Used for all built-in resource loading in libdonut.
    - Custom [Variant](include/donut/Variant.hpp) implementation:
        - Enhances the API of std::variant by adding the methods `is<T>()`, `as<T>()`, `get<T>()` and `get_if<T>()` as well as a freestanding `match` function.
        - Visitation is implemented through chaining of the conditional operator rather than virtual method dispatch, making it easier for compilers to generate a jump table than in most common std::variant implementations. This puts `match` on par with a raw switch statement in terms of performance.
        - Supports being used as a base class in user code.
    - [Loose Quadtree](include/donut/LooseQuadtree.hpp) container for accelerating AABB collision tests between a large number of objects in 2D.
    - [Time](include/donut/Time.hpp) duration wrapper including common utility functions for correctly handling discrete-time update loops.
    - Fast [pseudo-random number generation engine](include/donut/random.hpp) using [xoroshiro128++](https://prng.di.unimi.it/):
        - Designed to be used as a URBG in conjunction with the [standard C++ distributions](https://en.cppreference.com/w/cpp/numeric/random#Random_number_distributions).
    - [Compile-time reflection](include/donut/reflection.hpp) of aggregate types through some dark template magic.
        - Supports iterating references to the fields of an instance of any plain struct, with full type information available.
        - Used in libdonut to automate JSON serialization, vertex attribute setup, etc.
    - Basic geometric [shapes](include/donut/shapes.hpp) with intersection tests.

## Authors

- Donut the Vikingchap: https://steamcommunity.com/id/donutvikingchap/

## Documentation

The API reference for the latest version of libdonut is available here: https://donutvikingchap.github.io/libdonut/index.html

## Prerequisites

The following programs need to be installed in order to build an application using libdonut:

- [CMake](https://cmake.org/) (version 3.21+).
- A C++20-compatible compiler, such as:
    - [GCC](https://gcc.gnu.org/) (version 12+), or
    - [Clang](https://clang.llvm.org/) (version 17+), or
    - [MSVC](https://visualstudio.microsoft.com/) (VS 2022 version 17+).

## Project setup

Follow these steps to create a new application project using libdonut:

1. Copy the contents of the included [template/](template) directory into a newly created folder for your project.
2. Edit the `CMakeLists.txt` file in your new project folder as follows:
    - Change the project name at the top of the file.
    - Optional, but recommended: Set the GIT_TAG to the commit hash of the libdonut release that you want to use instead of `"origin/main"`.
    - Optional: Change the value of APP_TARGET_NAME from `"app"` to your desired application executable name.
3. Use CMake to configure the project. This can be done by running the following commands from within your new project folder:
    ```sh
    mkdir build
    cd build
    cmake ..
    ```
    CMake will then use FetchContent to automatically download a copy of libdonut and all of the required dependencies, and save them under `build/_deps/`.

## Building

After performing the configuration steps described above, the project can be built from the `build/` folder at any time using the following command to compile in debug mode:

```sh
cmake --build . --config Debug
```

or the following command to compile in performance-optimized release mode instead:

```sh
cmake --build . --config Release
```

The resulting application executable is written to `build/bin/`.

### Adding source files

When adding new source files to your project, make sure to also add their filepaths to the `add_executable` command in `CMakeLists.txt`, before or after `"src/main.cpp"`, and reconfigure CMake with the `cmake ..` command. Otherwise, the new files will not be included in the build.

## Debugging

When running the compiled executable, make sure that the current working directory is set to point to the folder where your main data directory is located, if you have one. Otherwise, your application will likely fail to run due to missing resources.

## Generating the documentation

If you have [Doxygen](https://www.doxygen.nl/) installed, you can build a local copy of the [API reference for libdonut](https://donutvikingchap.github.io/libdonut/index.html) as follows.

After configuring your new project, run the following command from inside the `build/` directory to generate the documentation for libdonut's API:

```sh
cmake --build . --target donut-generate-documentation
```

The generated HTML file at `build/_deps/donut-build/docs/html/index.html` can then be opened in your favorite web browser to read the documentation and navigate it like a website.

## Distribution

After compiling your project in release mode, the resulting application executable can be packaged into a folder along with its main data directory and be distributed according to the relevant licenses mentioned below. A sample `copyright.txt` file is provided in the project template to illustrate the copyright notices that must be included when distributing an application containing the code of libdonut and its direct dependencies. Note that this is only an example and not any form of legal advice.

## License

Libdonut is distributed under the **MIT License**. See the included [LICENSE](LICENSE) file for more information.

### External libraries

Libdonut depends on the C++ standard library, for which any C++20-compatible implementation may be used, as well as the following third-party libraries, each one having its own license, with some including further dependencies:

- [{fmt}](https://github.com/fmtlib/fmt) (MIT License)
- [glad](https://github.com/Dav1dde/glad) (glad/gl.h is Public Domain, khrplatform.h is under MIT License)
- [GLM](https://github.com/g-truc/glm) (MIT License)
- [libschrift](https://github.com/tomolt/libschrift) (ISC License)
- [PhysicsFS](https://github.com/icculus/physfs) (zlib License)
- [SDL](https://github.com/libsdl-org/SDL) (zlib License)
- [SoLoud](https://github.com/jarikomppa/soloud) (zlib License)
- [stb](https://github.com/nothings/stb) (Public Domain)

For graphics, libdonut expects the end user to have an available graphics driver installed that implements the [OpenGL](https://www.khronos.org/opengl/) Graphics System (Version 3.3 (Core Profile)) [specification](https://registry.khronos.org/OpenGL/specs/gl/glspec33.core.pdf). The graphics library is loaded at runtime through glad, using the function loader provided by SDL.
