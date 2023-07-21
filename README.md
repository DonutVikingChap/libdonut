# libdonut

Libdonut is an application framework for cross-platform game development in C++20.

Note: This library is currently a **work in progress** and makes no guarantees regarding future version compatibility.

## Features

- Application framework:
    - [Application](include/donut/application/Application.hpp) base class:
        - Controls the main game loop, including [Event](include/donut/application/Event.hpp) pumping, frame pacing and fixed-interval frame rate-independent tick updates.
        - Supports both native Windows/Linux compilation as well as the [emscripten](https://emscripten.org/) WebAssembly runtime.
    - [InputManager](include/donut/application/InputManager.hpp):
        - Maps physical [Input](include/donut/application/Input.hpp) events to abstract action numbers.
        - Supports simultaneous keyboard, mouse and controller input.
- Audio engine using [SoLoud](http://solhsa.com/soloud/):
    - [Sound](include/donut/audio/Sound.hpp) loading:
        - Support for OGG, WAV, FLAC and MP3 formats.
    - [SoundStage](include/donut/audio/SoundStage.hpp):
        - Plays 3D positional audio, background sounds and music.
- Graphics rendering through [OpenGL](https://www.khronos.org/opengl/):
    - [Window](include/donut/graphics/Window.hpp) abstraction:
        - Handles GL context setup and window management using [SDL](https://www.libsdl.org/).
    - [RenderPass](include/donut/graphics/RenderPass.hpp) interface for simple batch rendering:
        - 3D Model rendering with built-in Blinn-Phong lighting or custom shaders through [Shader3D](include/donut/graphics/Shader3D.hpp).
        - 2D Textured quad rendering with built-in or custom shaders through [Shader2D](include/donut/graphics/Shader2D.hpp).
        - Sprite rendering with automatic [SpriteAtlas](include/donut/graphics/SpriteAtlas.hpp) packing.
        - Text rendering and [Font](include/donut/graphics/Font.hpp) loading using [libschrift](https://github.com/tomolt/libschrift).
    - [Renderer](include/donut/graphics/Renderer.hpp) supporting arbitrary [Framebuffer](include/donut/graphics/Framebuffer.hpp) targets, [Camera](include/donut/graphics/Camera.hpp) positions and [Viewport](include/donut/graphics/Viewport.hpp) areas with optional integer scaling.
    - [Model](include/donut/graphics/Model.hpp) loading from OBJ files.
    - [Image](include/donut/graphics/Image.hpp) loading/saving using [stbi](https://github.com/nothings/stb).
- Utilities:
    - [AtlasPacker](include/donut/AtlasPacker.hpp) for packing rectangles into expandable square texture atlases.
    - Floating-point RGBA [Color](include/donut/Color.hpp) type that includes predefined constants for common web colors.
    - Virtual [File](include/donut/File.hpp) system with streamed [reading](include/donut/InputFileStream.hpp) and [writing](include/donut/OutputFileStream.hpp) using [PhysicsFS](https://icculus.org/physfs/):
        - Supports automatic mounting of pak/zip/etc. archives at startup for easy mod loading.
        - Used for all built-in resource loading in libdonut.
    - Custom [Variant](include/donut/Variant.hpp) implementation:
        - Provides a cleaner API than std::variant by adding the methods `is<T>()`, `as<T>()`, `get<T>()` and `get_if<T>()` as well as a freestanding `match` function.
        - Visitation is implemented through chaining of the conditional operator rather than virtual method dispatch, making it easier for compilers to generate a jump table than in most common std::variant implementations. This puts `match` on par with a raw switch statement in terms of performance.
        - Supports being used as a base class in user code.
    - [Time](include/donut/Time.hpp) duration wrapper including common utility functions for correctly handling discrete-time update loops.
    - [Base64](include/donut/base64.hpp) encoding and decoding.
    - [JSON](include/donut/json.hpp) utilities:
        - Writing/parsing to/from strings and iostreams.
        - Extensible serialization/deserialization of arbitrary types.
        - Supports [JSON5](https://json5.org/) features (comments, trailing commas, identifier keys, etc.).
    - [Loose Quadtree](include/donut/LooseQuadtree.hpp) container for accelerating AABB collision tests between a large number of objects in 2D.
    - [OBJ](include/donut/obj.hpp) model/material parsing.
    - Fast [pseudo-random number generation engine](include/donut/random.hpp) using [xoroshiro128++](https://prng.di.unimi.it/):
        - Designed to be used as a URBG in conjunction with the [standard C++ distributions](https://en.cppreference.com/w/cpp/numeric/random#Random_number_distributions).
    - [Compile-time reflection](include/donut/reflection.hpp) of aggregate types through some dark template magic.
        - Supports iterating references to the fields of an instance of any plain struct, with full type information available.
        - Used in libdonut to automate JSON serialization, OpenGL vertex attribute setup, etc.
    - Basic geometric [shapes](include/donut/shapes.hpp) with intersection tests.
    - Utilities for working with [UTF-8](include/donut/unicode.hpp)-encoded text:
        - Supports iterating unicode code points in any arbitrary byte sequence with basic error handling in the case of invalid encoding.
    - [XML](include/donut/xml.hpp) document parsing.

## Authors

- Donut the Vikingchap: https://steamcommunity.com/id/donutvikingchap/

## Prerequisites

The following programs need to be installed in order to build an application using libdonut:

- [CMake](https://cmake.org/) (version 3.15+).
- A C++20-compatible compiler, such as:
    - [GCC](https://gcc.gnu.org/) (version 13+), or
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

## Documentation

The API reference for the latest version of libdonut is available here: https://donutvikingchap.github.io/libdonut/index.html

### Generating manually

If you have [Doxygen](https://www.doxygen.nl/) installed, you can also build the API reference for libdonut as follows.

After configuring your new project, run the following command from inside the `build/` directory to generate the documentation for libdonut's API:

```sh
cmake --build . --target donut-generate-documentation
```

The generated HTML file at `build/_deps/donut-build/docs/html/index.html` can then be opened in your favorite web browser to read the documentation and navigate it like a website.

## Distribution

After compiling in release mode, the resulting application executable can be packaged into a folder along with its main data directory and be distributed according to each of the relevant licenses mentioned below. A sample `copyright.txt` file is provided in the project template to illustrate the copyright notices that must be included when distributing an application containing the code of libdonut and its direct dependencies. Note however that this is only an example and not any form of legal advice.

## License

Libdonut is distributed under the **MIT License**. See the included [LICENSE](LICENSE) file for more information.

### External libraries

Libdonut depends on the C++ standard library, for which any C++20-compatible implementation may be used, as well as the following specific third-party libraries, each one having its own license, with some including further dependencies:

- [glad](https://github.com/Dav1dde/glad) (glad/gl.h is Public Domain, khrplatform.h is under MIT License)
- [GLM](https://github.com/g-truc/glm) (MIT License)
- [libschrift](https://github.com/tomolt/libschrift) (ISC License)
- [PhysicsFS](https://github.com/icculus/physfs) (zlib License)
- [SDL](https://github.com/libsdl-org/SDL) (zlib License)
- [SoLoud](https://github.com/jarikomppa/soloud) (zlib License)
- [stb](https://github.com/nothings/stb) (Public Domain)

For graphics, libdonut expects the end user to have an available graphics driver installed that implements the [OpenGL](https://www.khronos.org/opengl/) Graphics System (Version 3.3 (Core Profile)) as [specified](https://registry.khronos.org/OpenGL/specs/gl/glspec33.core.pdf) by The Khronos Group Inc. The GL library is loaded at runtime through glad, using the function loader provided by SDL.
