![Developed by atmc Badge](https://img.shields.io/badge/atmc-We%20made%20this!-%23303030?labelColor=%23c80000)

# libcxxext

A system library for C++, used as an extension of the existing standard library.

## Overview

### Infrastructure

This project provides some universal tooling for C++, for upstream consumption.

> [!NOTE]  
> Upstream targets: Depend on the copies of Catch2 and rapidcheck here.

CMake includes are under `cmake/`.

| Script                |                                                                            |
| --------------------- | -------------------------------------------------------------------------- |
| `build_support.cmake` | Linkable CMake utility targets.                                            |
| `catch2.cmake`        | Create unit and property-based / fuzz tests.                               |
| `clang_tidy.cmake`    | Check target sources with clang-tidy. Don't forget to pass in headers too! |

### `libcxxext` Targets

| Library          |                                     |
| ---------------- | ----------------------------------- |
| `sys`            | Basic core functions and utilities. |
| `sys.Containers` | Additional collections for data.    |
| `sys.Threading`  | Generic thread support and tasks.   |

> [!TIP]  
> Example and test targets are automatically generated from `.cpp` files under `examples/` and `tests/`.
