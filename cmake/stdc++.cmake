# Assumes at least C++17, C++20 onwards is conditionally supported per dependent target.

add_library(sys.BuildSupport.stdc++inc INTERFACE)
target_precompile_headers(sys.BuildSupport.stdc++inc INTERFACE
    # Multi-Purpose Headers
    <cstdlib>
    <execution>

    # Language Support
    <cfloat>
    <climits>
    $<$<COMPILE_FEATURES:cxx_std_20>:<compare>>
    $<$<COMPILE_FEATURES:cxx_std_20>:<coroutine>>
    <csetjmp>
    <csignal>
    <cstdarg>
    <cstddef>
    <cstdint>
    <exception>
    <initializer_list>
    <limits>
    <new>
    $<$<COMPILE_FEATURES:cxx_std_20>:<source_location>>
    $<$<COMPILE_FEATURES:cxx_std_23>:<stdfloat>>
    <typeindex>
    <typeinfo>
    $<$<COMPILE_FEATURES:cxx_std_20>:<version>>

    # Concepts
    $<$<COMPILE_FEATURES:cxx_std_20>:<concepts>>

    # Diagnostics
    <cassert>
    <cerrno>
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_26>:<debugging>>
    $<$<COMPILE_FEATURES:cxx_std_23>:<stacktrace>>
    <stdexcept>
    <system_error>

    # Memory Management
    <memory>
    <memory_resource>
    <scoped_allocator>

    # Metaprogramming
    <ratio>
    <type_traits>

    # General Utilities
    <any>
    $<$<COMPILE_FEATURES:cxx_std_20>:<bit>>
    <bitset>
    $<$<COMPILE_FEATURES:cxx_std_23>:<expected>>
    <functional>
    <optional>
    <tuple>
    <utility>
    <variant>

    # Containers
    <array>
    <deque>
    $<$<COMPILE_FEATURES:cxx_std_23>:<flat_map>>
    $<$<COMPILE_FEATURES:cxx_std_23>:<flat_set>>
    <forward_list>
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_26>:<hive>>
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_26>:<inplace_vector>>
    <list>
    <map>
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_23>:<mdspan>>
    <queue>
    <set>
    $<$<COMPILE_FEATURES:cxx_std_20>:<span>>
    <stack>
    <unordered_map>
    <unordered_set>
    <vector>

    # Iterators
    <iterator>

    # Ranges
    $<$<COMPILE_FEATURES:cxx_std_23>:<generator>>
    $<$<COMPILE_FEATURES:cxx_std_20>:<ranges>>

    # Algorithms
    <algorithm>
    <numeric>

    # Strings
    <cstring>
    <string>
    <string_view>

    # Text processing
    <cctype>
    <charconv>
    <clocale>
    <cuchar>
    <cwchar>
    <cwctype>
    $<$<COMPILE_FEATURES:cxx_std_20>:<format>>
    <locale>
    <regex>
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_26>:<text_encoding>>

    # Numerics
    <cfenv>
    <cmath>
    <complex>
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_26>:<linalg>>
    $<$<COMPILE_FEATURES:cxx_std_20>:<numbers>>
    <random>
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_26>:<simd>>
    <valarray>

    # Time
    <chrono>
    <ctime>

    # Input/output
    <cinttypes>
    <cstdio>
    <filesystem>
    <fstream>
    <iomanip>
    <ios>
    <iosfwd>
    <iostream>
    <istream>
    <ostream>
    $<$<COMPILE_FEATURES:cxx_std_23>:<print>>
    $<$<COMPILE_FEATURES:cxx_std_23>:<spanstream>>
    <sstream>
    <streambuf>
    $<$<COMPILE_FEATURES:cxx_std_20>:<syncstream>>

    # Concurrency
    <atomic>
    $<$<COMPILE_FEATURES:cxx_std_20>:<barrier>>
    <condition_variable>
    <future>
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_26>:<hazard_pointer>>
    $<$<COMPILE_FEATURES:cxx_std_20>:<latch>>
    <mutex>
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_26>:<rcu>>
    $<$<COMPILE_FEATURES:cxx_std_20>:<semaphore>>
    <shared_mutex>
    $<$<COMPILE_FEATURES:cxx_std_20>:<stop_token>>
    <thread>
)
