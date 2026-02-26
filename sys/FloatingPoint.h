#pragma once

#if __has_include(<stdfloat>)
#include <stdfloat> // NOLINT(misc-include-cleaner)
#endif

/// @defgroup floating_point Floating Point Types
/// @brief Fixed-width floating point types.
/// @note Pass `byval`.
/// @{

#if defined(__STDCPP_FLOAT32_T__) && __STDCPP_FLOAT32_T__
using f32 = std::float32_t;
#else
using f32 = float; /**< 32-bit floating point type. */
#endif
#if defined(__STDCPP_FLOAT64_T__) && __STDCPP_FLOAT64_T__
using f64 = std::float64_t;
#else
using f64 = double; /**< 64-bit floating point type. */
#endif

/// @}
