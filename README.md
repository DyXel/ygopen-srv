# YGOpen Legacy Server

The YGOpen project aims to create a free-as-in-freedom, cleanly-engineered Yu-Gi-Oh! Official Card Game simulator, complete with client and server support. This server supports [EDOPro](https://github.com/edo9300/ygopro) clients and dynamically links to [that core](https://github.com/edo9300/ygopro-core).

## Prerequisites

- Git (or you can download the zip from GitHub)
- [premake5](https://premake.github.io/download.html#v5): extract this into the working directory
- Platform C++ toolchain
  - Windows: [Visual Studio](https://visualstudio.microsoft.com/)
  - macOS: GCC (Clang does NOT support C++14/C++17 standard library)
  - Linux: GCC or Clang

- On macOS, we use [Homebrew](https://brew.sh/) to get dependencies. Install `gcc` with `brew install gcc`
- On Windows, set up [`vcpkg`](https://github.com/microsoft/vcpkg)

## Dependencies

Build [ocgcore](https://github.com/edo9300/ygopro-core) as a shared library and put it in the folder where you will move the server executable.

On Linux, get these dependencies from your package manager or build from source: `asio nlohmann-json sqlite3`.

On Windows, invoke vcpkg to build dependencies from source: `vcpkg install --triplet x86-windows-static asio nlohmann-json sqlite3`.

On macOS, install dependencies with `brew install asio nlohmann-json sqlite3`.

## Build

On Linux, do `./premake5 gmake2 && make -Cbuild` for a debug build, or `make -Cbuild config=release` for a release build.

On Windows, do `./premake5.exe vs2017` or `./premake5.exe vs2019` to create the solution files and then build from the generated Visual Studio `.sln` file in `build`.

On macOS, do `./premake5 gmake2 && make -Cbuild CC=gcc-9 CXX=g++-9` for a GCC debug build, assuming you installed GCC with brew.
