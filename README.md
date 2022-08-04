# dawn

A scrappy compiler middle and back-end library.

## Introduction

Dawn is a compiler middle-end and back-end library, heavily inspired by the
design (and implementation) of LLVM. The main difference is that it's made by
a single person and made entirely for learning purposes (and fun!).

## Building

See the [BUILDING](BUILDING.md) document.

Useful CMake options (all `OFF` by default):

- `DAWN_DEVELOPER_MODE`: Whether to build with any developer targets enabled.
- `DAWN_BUILD_BENCHMARKS`: Whether to build benchmarks or not

## Licensing

Licensed under the Apache-2.0 license. See [LICENSE.txt](./LICENSE.txt)