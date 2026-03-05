![Developed by atmc Badge](https://img.shields.io/badge/atmc-We%20made%20this!-%23303030?labelColor=%23c80000)

# libcxxext

A system library for C++, used as an extension of the existing standard library.

## Building or Using `libcxxext`

### Building for Binary Distribution

Running:

`cmake -Bbuild -DCMAKE_INSTALL_PREFIX=install`  
`cmake --build build`  
`cmake --build build --target install`  
`cd build && cpack`

should just work, and give you an archive with `bin/`, `lib/`, and `include/`.

### Using

The recommended way to use `libcxxext` is to add it as a subdirectory.

```cmake
set(LIBCXXEXT_DEVELOPMENT_MODE ... CACHE INTERNAL "")
set(CMAKE_INSTALL_PREFIX ... CACHE INTERNAL "")
set(LIBCXXEXT_EXAMPLES ... CACHE INTERNAL "")
set(LIBCXXEXT_TESTS ... CACHE INTERNAL "")
set(LIBCXXEXT_COVERAGE ... CACHE INTERNAL "")
set(LIBCXXEXT_LIBRARY_TYPE ... CACHE INTERNAL "")
set(LIBCXXEXT_EXPORT_PCH ... CACHE INTERNAL "")
set(LIBCXXEXT_TEST_ASAN ... CACHE INTERNAL "")
set(LIBCXXEXT_TEST_UBSAN ... CACHE INTERNAL "")
add_subdirectory(path/to/libcxxext)
...
target_link_libraries(my_target PRIVATE sys ...)
```

## Overview

### Infrastructure

This project provides some universal tooling for C++, for upstream consumption.

> [!NOTE]  
> Upstream targets: Depend on the copies of Catch2, rapidcheck, and doxygen-awesome-css here.

CMake includes are under `cmake/`.

| Script                      |                                                                            |
| --------------------------- | -------------------------------------------------------------------------- |
| `build_support.cmake`       | Linkable CMake utility targets.                                            |
| `catch2.cmake`              | Create unit and property-based / fuzz tests.                               |
| `clang_tidy.cmake`          | Check target sources with clang-tidy. Don't forget to pass in headers too! |
| `workflows/all_push.py`     | Presubmit on-push workflow running checks described in some `checks.json`. |
| `workflows/all_findings.py` | Presubmit findings aggregator workflow printing markdown to `stdout`.      |
| `workflows/cppconf.py`      | CMake C/C++ configure project check.                                       |
| `workflows/cppbuild.py`     | CMake C/C++ build project check.                                           |
| `workflows/cppfmt.py`       | C/C++ `clang-format` check.                                                |
| `workflows/cpplint.py`      | CMake C++ linting check for projects using `clang_tidy.cmake`.             |
| `workflows/cpptest.py`      | CTest + coverage info (with `sys.BuildSupport.EnableCoverage`) check.      |
| `workflows/cppcov.py`       | Coverage report generator for after CTest.                                 |
| `workflows/cpppack.py`      | CPack check.                                                               |
| `workflows/pylint.py`       | Python linting check.                                                      |
| `workflows/docgen.py`       | Doxygen documentation generator check.                                     |
| `workflows/template.py`     | Workflow check template.                                                   |

### Libraries

> [!TIP]  
> In `libcxxext`, example and test targets are automatically generated from `.cpp` files under `examples/` and `tests/`.

| Library                               |                                                                                           |
| ------------------------------------- | ----------------------------------------------------------------------------------------- |
| `sys.BuildSupport.CompilerOptions`    | Interface target for an opinionated set of compiler options, if supported.                |
| `sys.BuildSupport.WarningsAsErrors`   | Interface target for treating compiler warnings as errors, if supported.                  |
| `sys.BuildSupport.Hardening`          | Interface target for compiler hardening flags, if supported.                              |
| `sys.BuildSupport.EnableCoverage`     | Interface target for enabling coverage instrumentation and data generation, if supported. |
| `sys.BuildSupport.AddressSanitizer`   | Interface target for enabling `asan`, if supported.                                       |
| `sys.BuildSupport.UndefinedSanitizer` | Interface target for enabling `ubsan`, if supported.                                      |
| `sys`                                 | Basic core functions and utilities.                                                       |
| `sys.Containers`                      | Additional collections for data.                                                          |
| `sys.Text`                            | Unicode-encoded string functionality.                                                     |
| `sys.Threading`                       | Generic thread support and tasks.                                                         |

## See Also

|                        |                               |
| ---------------------- | ----------------------------- |
| License                | [LICENSE](LICENSE)            |
| C/C++ Style Guide      | [codestyle.md](codestyle.md)  |
| C/C++ Formatting Rules | [clang-format](.clang-format) |
| C/C++ Linting Rules    | [clang-tidy](.clang-tidy)     |
| Presubmit Checksets    | [checks.json](checks.json)    |

### ... See Even More

|                                 |                                        |
| ------------------------------- | -------------------------------------- |
| Presubmit Check Scripts         | [workflows](workflows)                 |
| Presubmit `.py` Support Library | [workflows/lib](workflows/lib)         |
| General-Purpose Scripts         | [workflows/scripts](workflows/scripts) |
| Presubmit Check Tool Support    | [workflows/tools](workflows/tools)     |

## Project Metadata

| Field     | Value                                                                            |
| --------- | -------------------------------------------------------------------------------- |
| `name`    | libcxxext                                                                        |
| `version` | 1.0.0                                                                            |
| `desc`    | A system library for C++, used as an extension of the existing standard library. |
