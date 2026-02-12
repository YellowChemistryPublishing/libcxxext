# Assumes at least C++17, C++20 onwards is conditionally supported per dependent target.

add_library(sys.BuildSupport.stdc++inc INTERFACE)
target_precompile_headers(sys.BuildSupport.stdc++inc INTERFACE
    # Multi-Purpose Headers
    <cstdlib>
    <execution>

    # Language Support
    <cfloat>
    <climits>
    $<$<COMPILE_FEATURES:cxx_std_20>:$<1:<compare>>>
    $<$<COMPILE_FEATURES:cxx_std_20>:$<1:<coroutine>>>
    <csetjmp>
    <csignal>
    <cstdarg>
    <cstddef>
    <cstdint>
    <exception>
    <initializer_list>
    <limits>
    <new>
    $<$<COMPILE_FEATURES:cxx_std_20>:$<1:<source_location>>>
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_23>:$<1:<stdfloat>>>
    <typeindex>
    <typeinfo>
    $<$<COMPILE_FEATURES:cxx_std_20>:$<1:<version>>>

    # Concepts
    $<$<COMPILE_FEATURES:cxx_std_20>:$<1:<concepts>>>

    # Diagnostics
    <cassert>
    <cerrno>
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_26>:$<1:<debugging>>>
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_23>:$<1:<stacktrace>>>
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
    $<$<COMPILE_FEATURES:cxx_std_20>:$<1:<bit>>>
    <bitset>
    $<$<COMPILE_FEATURES:cxx_std_23>:$<1:<expected>>>
    <functional>
    <optional>
    <tuple>
    <utility>
    <variant>

    # Containers
    <array>
    <deque>
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_23>:$<1:<flat_map>>>
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_23>:$<1:<flat_set>>>
    <forward_list>
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_26>:$<1:<hive>>>
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_26>:$<1:<inplace_vector>>>
    <list>
    <map>
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_23>:$<1:<mdspan>>>
    <queue>
    <set>
    $<$<COMPILE_FEATURES:cxx_std_20>:$<1:<span>>>
    <stack>
    <unordered_map>
    <unordered_set>
    <vector>

    # Iterators
    <iterator>

    # Ranges
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_23>:$<1:<generator>>>
    $<$<COMPILE_FEATURES:cxx_std_20>:$<1:<ranges>>>

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
    # Broken on MSVC: <cuchar>
    <cwchar>
    <cwctype>
    $<$<COMPILE_FEATURES:cxx_std_20>:$<1:<format>>>
    <locale>
    <regex>
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_26>:$<1:<text_encoding>>>

    # Numerics
    <cfenv>
    <cmath>
    <complex>
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_26>:$<1:<linalg>>>
    $<$<COMPILE_FEATURES:cxx_std_20>:$<1:<numbers>>>
    # random
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_26>:$<1:<simd>>>
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
    $<$<COMPILE_FEATURES:cxx_std_23>:$<1:<print>>>
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_23>:$<1:<spanstream>>>
    <sstream>
    <streambuf>
    $<$<COMPILE_FEATURES:cxx_std_20>:$<1:<syncstream>>>

    # Concurrency
    <atomic>
    $<$<COMPILE_FEATURES:cxx_std_20>:$<1:<barrier>>>
    # condition_variable
    # future
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_26>:$<1:<hazard_pointer>>>
    $<$<COMPILE_FEATURES:cxx_std_20>:$<1:<latch>>>
    # mutex
    # TODO: Once more available, add $<$<COMPILE_FEATURES:cxx_std_26>:$<1:<rcu>>>
    $<$<COMPILE_FEATURES:cxx_std_20>:$<1:<semaphore>>>
    # shared_mutex
    $<$<COMPILE_FEATURES:cxx_std_20>:$<1:<stop_token>>>
    <thread>
)
