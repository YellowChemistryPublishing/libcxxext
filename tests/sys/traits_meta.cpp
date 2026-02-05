#include <concepts>
#include <functional>
#include <iterator>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

// NOLINTBEGIN(misc-include-cleaner)

#include <catch2/catch_all.hpp>

#include <module/sys>

TEST_CASE("Invariably passes.", "[sys][traits][meta]") { CHECK(true); }

struct test_class
{
    // NOLINTBEGIN(readability-convert-member-functions-to-static)
    [[nodiscard]] int member_func(float, double) { return 0; }
    [[nodiscard]] int const_member_func(float) const { return 0; }
    // NOLINTEND(readability-convert-member-functions-to-static)

    static int static_func(int) { return 0; }
};

// ================================================================================
// Regular function types. | `sys::meta::function_signature<T(Args...)>`
// ================================================================================

// `return_type` is correctly deduced.
static_assert(std::same_as<sys::meta::function_signature<int(float, double)>::return_type, int>);

// `void` return type works.
static_assert(std::same_as<sys::meta::function_signature<void()>::return_type, void>);

// `arguments` tuple is correctly deduced.
static_assert(std::same_as<sys::meta::function_signature<int(float, double, char)>::arguments, std::tuple<float, double, char>>);

// Empty arguments works.
static_assert(std::same_as<sys::meta::function_signature<void()>::arguments, std::tuple<>>);

// `is_member_func()` returns `false` for regular functions.
static_assert(!sys::meta::function_signature<int(float)>::is_member_func());

// `is_signature_of()` matches compatible function pointers.
static_assert(sys::meta::function_signature<int(int, float)>::is_signature_of<int (*)(int, float)>());

// `is_signature_of()` rejects incompatible functors.
static_assert(!sys::meta::function_signature<int(int, float)>::is_signature_of<void (*)()>());
static_assert(!sys::meta::function_signature<int(int, float)>::is_signature_of<int (*)(int)>());

// ================================================================================
// Function pointer types. | `sys::meta::function_signature<T(*)(Args...)>`
// ================================================================================

// `return_type` is correctly deduced.
static_assert(std::same_as<sys::meta::function_signature<int (*)(float, double)>::return_type, int>);

// `arguments` tuple is correctly deduced.
static_assert(std::same_as<sys::meta::function_signature<int (*)(float, double)>::arguments, std::tuple<float, double>>);

// `is_member_func()` returns `false` for function pointers.
static_assert(!sys::meta::function_signature<int (*)(float)>::is_member_func());

// ================================================================================
// Member function pointer types. | `sys::meta::function_signature<T(For::*)(Args...)>`
// ================================================================================

// `return_type` is correctly deduced.
static_assert(std::same_as<sys::meta::function_signature<int (test_class::*)(float, double)>::return_type, int>);

// `arguments` tuple is correctly deduced.
static_assert(std::same_as<sys::meta::function_signature<int (test_class::*)(float, double)>::arguments, std::tuple<float, double>>);

// `is_member_func()` returns `true` for member function pointers.
static_assert(sys::meta::function_signature<int (test_class::*)(float, double)>::is_member_func());

// Works with actual member function pointers via `decltype`.
static_assert(sys::meta::function_signature<decltype(&test_class::member_func)>::is_member_func());
static_assert(std::same_as<sys::meta::function_signature<decltype(&test_class::member_func)>::return_type, int>);

// ================================================================================
// Fallback for non-function types. | `sys::meta::function_signature<InvalidType>`
// ================================================================================

// `is_signature_of()` returns `false` for non-function types.
static_assert(!sys::meta::function_signature<int>::is_signature_of<void (*)()>());

// Works with struct types.
static_assert(!sys::meta::function_signature<test_class>::is_signature_of<int (*)(int)>());

// ================================================================================
// Basic operations. | `sys::meta::parameter_pack`
// ================================================================================

using test_pack_meta = sys::meta::parameter_pack<int, float, double, char>;

// `tuple` type alias works.
static_assert(std::same_as<test_pack_meta::tuple, std::tuple<int, float, double, char>>);

// `at<Index>` returns correct type.
static_assert(std::same_as<test_pack_meta::at<0>, int>);
static_assert(std::same_as<test_pack_meta::at<1>, float>);
static_assert(std::same_as<test_pack_meta::at<2>, double>);
static_assert(std::same_as<test_pack_meta::at<3>, char>);

// `contains<T>()` returns `true` for types in pack.
static_assert(test_pack_meta::contains<int>());
static_assert(test_pack_meta::contains<float>());
static_assert(test_pack_meta::contains<double>());
static_assert(test_pack_meta::contains<char>());

// `contains<T>()` returns `false` for types not in pack.
static_assert(!test_pack_meta::contains<long>()); // NOLINT(google-runtime-int)
static_assert(!test_pack_meta::contains<std::string>());
static_assert(!test_pack_meta::contains<void>());

// ================================================================================
// Edge cases. | `sys::meta::parameter_pack`
// ================================================================================

// Empty pack works.
using empty_pack = sys::meta::parameter_pack<>;
static_assert(std::same_as<empty_pack::tuple, std::tuple<>>);
static_assert(!empty_pack::contains<int>());

// Single element pack works.
using single_pack = sys::meta::parameter_pack<int>;
static_assert(std::same_as<single_pack::at<0>, int>);
static_assert(single_pack::contains<int>());
static_assert(!single_pack::contains<float>());

// Duplicate types in pack.
using dups_pack = sys::meta::parameter_pack<int, int, int>;
static_assert(dups_pack::contains<int>());
static_assert(std::same_as<dups_pack::at<0>, int>);
static_assert(std::same_as<dups_pack::at<2>, int>);

// NOLINTEND(misc-include-cleaner)
