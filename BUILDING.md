# Building with CMake

## Useful CMake Options

Useful feature switches for Dawn (all `OFF` by default):

- `DAWN_DEVELOPER_MODE`: Whether to build with any developer targets enabled.
- `DAWN_BUILD_TESTS`: Whether to build tests or not
- `DAWN_BUILD_BENCHMARKS`: Whether to build benchmarks or not
- `DAWN_INCLUDES_WITH_SYSTEM`: Whether to make Dawn make all includes be marked as `SYSTEM` includes

Useful Developer Variables:

- `DAWN_CXX_CLANG_TIDY`: Effectively `CXX_CLANG_TIDY` but for Dawn targets, can be used to set a custom `clang-tidy`
  binary and add extra flags (e.g. `--use-color` for an IDE).
- `DAWN_FORMAT_COMMAND`: The command used by the `format-*` targets. Can be set to a custom `clang-format` binary.
- `DAWN_CXX_FLAGS`: Mostly set by CMake profiles, set as compiler options for every Dawn target. Used for things
  like `-fsanitize` options or warning flags
- `DAWN_EXTRA_CXX_FLAGS`: Can be used to add additional compiler flags to Dawn targets, this is not specified by any of
  the normal CMake presets.

## Build

This project doesn't require any special command-line flags to build normally,
just make sure that the `vendor/abseil-cpp` Git submodule is checked out.

```sh
cmake -S . -B build -D CMAKE_BUILD_TYPE=Release
cmake --build build
```

Here are the steps for building in release mode with a multi-configuration
generator, like the Visual Studio ones:

```sh
cmake -S . -B build
cmake --build build --config Release
```

### Building with MSVC

Note that MSVC by default is not standards compliant and you need to pass some
flags to make it behave properly. See the `flags-windows` preset in the
[CMakePresets.json](CMakePresets.json) file for the flags and with what
variable to provide them to CMake during configuration.

### Building on Apple Silicon

CMake supports building on Apple Silicon properly since 3.20.1. Make sure you
have the [latest version][1] installed.

## Install

This project doesn't require any special command-line flags to install to keep
things simple. As a prerequisite, the project has to be built with the above
commands already.

The below commands require at least CMake 3.15 to run, because that is the
version in which [Install a Project][2] was added.

Here is the command for installing the release mode artifacts with a
single-configuration generator, like the Unix Makefiles one:

```sh
cmake --install build
```

Here is the command for installing the release mode artifacts with a
multi-configuration generator, like the Visual Studio ones:

```sh
cmake --install build --config Release
```

### CMake package

This project exports a CMake package to be used with the [`find_package`][3]
command of CMake:

* Package name: `dawn`
* Target name: `dawn::dawn`

Example usage:

```cmake
find_package(dawn REQUIRED)
# Declare the imported target as a build requirement using PRIVATE, where
# project_target is a target created in the consuming project
target_link_libraries(
        project_target PRIVATE
        dawn::dawn
)
```

[1]: https://cmake.org/download/

[2]: https://cmake.org/cmake/help/latest/manual/cmake.1.html#install-a-project

[3]: https://cmake.org/cmake/help/latest/command/find_package.html
