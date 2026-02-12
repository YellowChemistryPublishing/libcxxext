#include "Traits.h"
#include <concepts>
#include <string>
#include <tuple>

// NOLINTBEGIN(misc-include-cleaner)

#include <catch2/catch_all.hpp>

#include <module/sys>

namespace
{

    struct append_receiver
    {
        int val = 0; // NOLINT(misc-non-private-member-variables-in-classes)
        void push_back(int v) { val = v; }
    };
} // namespace

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

// `.is_member_func()` returns `false` for regular functions.
static_assert(!sys::meta::function_signature<int(float)>::is_member_func());

// `.is_signature_of()` matches compatible function pointers.
static_assert(sys::meta::function_signature<int(int, float)>::is_signature_of<int (*)(int, float)>());

// `.is_signature_of()` rejects incompatible functors.
static_assert(!sys::meta::function_signature<int(int, float)>::is_signature_of<void (*)()>());
static_assert(!sys::meta::function_signature<int(int, float)>::is_signature_of<int (*)(int)>());

// ================================================================================
// Function pointer types. | `sys::meta::function_signature<T(*)(Args...)>`
// ================================================================================

// `return_type` is correctly deduced.
static_assert(std::same_as<sys::meta::function_signature<int (*)(float, double)>::return_type, int>);

// `arguments` tuple is correctly deduced.
static_assert(std::same_as<sys::meta::function_signature<int (*)(float, double)>::arguments, std::tuple<float, double>>);

// `.is_member_func()` returns `false` for function pointers.
static_assert(!sys::meta::function_signature<int (*)(float)>::is_member_func());

// ================================================================================
// Member function pointer types. | `sys::meta::function_signature<T (For::*)(Args...)>`
// ================================================================================

namespace
{
    struct test_class
    {
        // NOLINTBEGIN(readability-convert-member-functions-to-static)
        [[nodiscard]] int member_func(float, double) { return 0; }
        [[nodiscard]] int const_member_func(float) const { return 0; }
        // NOLINTEND(readability-convert-member-functions-to-static)

        static int static_func(int) { return 0; }
    };
} // namespace

// `return_type` is correctly deduced.
static_assert(std::same_as<sys::meta::function_signature<int (test_class::*)(float, double)>::return_type, int>);

// `arguments` tuple is correctly deduced.
static_assert(std::same_as<sys::meta::function_signature<int (test_class::*)(float, double)>::arguments, std::tuple<float, double>>);

// `.is_member_func()` returns `true` for member function pointers.
static_assert(sys::meta::function_signature<int (test_class::*)(float, double)>::is_member_func());

// Works with actual member function pointers via `decltype(...)`.
static_assert(sys::meta::function_signature<decltype(&test_class::member_func)>::is_member_func());
static_assert(std::same_as<sys::meta::function_signature<decltype(&test_class::member_func)>::return_type, int>);

// ================================================================================
// Reference overloads and cv-qualifiers. | `sys::meta::function_signature<...>`
// ================================================================================

namespace
{
    struct qual_ref
    {
        void const_func() const { }
        void volatile_func() volatile { }
        void cv_func() const volatile { }

        void const_ref_func() const& { }
        void volatile_ref_func() volatile& { }
        void cv_ref_func() const volatile& { }

        void const_rref_func() const&& { }
        void volatile_rref_func() volatile&& { }
        void cv_rref_func() const volatile&& { }
    };
} // namespace

static_assert(sys::meta::function_signature<decltype(&qual_ref::const_func)>::is_const());
static_assert(sys::meta::function_signature<decltype(&qual_ref::volatile_func)>::is_volatile());
static_assert(sys::meta::function_signature<decltype(&qual_ref::cv_func)>::is_const());
static_assert(sys::meta::function_signature<decltype(&qual_ref::cv_func)>::is_volatile());

static_assert(sys::meta::function_signature<decltype(&qual_ref::const_ref_func)>::is_const());
static_assert(sys::meta::function_signature<decltype(&qual_ref::const_ref_func)>::is_lvalue_ref());
static_assert(!sys::meta::function_signature<decltype(&qual_ref::const_ref_func)>::is_rvalue_ref());

static_assert(sys::meta::function_signature<decltype(&qual_ref::const_rref_func)>::is_const());
static_assert(!sys::meta::function_signature<decltype(&qual_ref::const_rref_func)>::is_lvalue_ref());
static_assert(sys::meta::function_signature<decltype(&qual_ref::const_rref_func)>::is_rvalue_ref());

// ================================================================================
// Fallback for non-function types. | `sys::meta::function_signature<InvalidType>`
// ================================================================================

// `.is_signature_of()` returns `false` for non-function types.
static_assert(!sys::meta::function_signature<int>::is_signature_of<void (*)()>());

// Works with struct types.
static_assert(!sys::meta::function_signature<test_class>::is_signature_of<int (*)(int)>());

// ================================================================================
// Basic operations. | `sys::meta::parameter_pack<...>`
// ================================================================================

using test_pack_meta = sys::meta::parameter_pack<int, float, double, char>;

// `tuple` type alias works.
static_assert(std::same_as<test_pack_meta::tuple, std::tuple<int, float, double, char>>);

// `at<...>` returns correct type.
static_assert(std::same_as<test_pack_meta::at<0>, int>);
static_assert(std::same_as<test_pack_meta::at<1>, float>);
static_assert(std::same_as<test_pack_meta::at<2>, double>);
static_assert(std::same_as<test_pack_meta::at<3>, char>);

// `.contains<...>()` returns `true` for types in pack.
static_assert(test_pack_meta::contains<int>());
static_assert(test_pack_meta::contains<float>());
static_assert(test_pack_meta::contains<double>());
static_assert(test_pack_meta::contains<char>());

// `.contains<...>()` returns `false` for types not in pack.
static_assert(!test_pack_meta::contains<long>()); // NOLINT(google-runtime-int)
static_assert(!test_pack_meta::contains<std::string>());
static_assert(!test_pack_meta::contains<void>());

// ================================================================================
// Edge cases. | `sys::meta::parameter_pack<...>`
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
using multi_pack = sys::meta::parameter_pack<int, int, int>;
static_assert(multi_pack::contains<int>());
static_assert(std::same_as<multi_pack::at<0>, int>);
static_assert(std::same_as<multi_pack::at<2>, int>);

// ================================================================================
// Type switch. | `sys::meta::type_switch<...>`
// ================================================================================

using sys::meta::type_switch, sys::meta::type_switch_cases, sys::meta::type_case;

static_assert(type_switch_cases<type_case<std::same_as<int, int>, int>, type_case<std::same_as<int, int>, bool>>::count_returns() == 2uz);
static_assert(type_switch_cases<type_case<std::same_as<int, bool>, int>, type_case<std::same_as<bool, int>, bool>>::count_returns() == 0uz);

static_assert(
    std::same_as<type_switch<type_case<std::same_as<size_t, wchar_t>, long double>, type_case<std::same_as<int, int>, int>, type_case<std::same_as<int, float>, float>>, int>);
static_assert(std::same_as<type_switch<type_case<std::same_as<float, double>, float>, type_case<false, bool>, type_case<true, void>>, void>);

// ================================================================================
// Container appending logic. | `sys::meta::append_to<...>(...)`
// ================================================================================

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("`sys::meta::append_to` works for various containers.", "[sys][traits][meta][append_to]")
{
    SECTION("`std::vector<...>`")
    {
        std::vector<int> vec;
        sys::meta::append_to(vec, 1);
        CHECK(vec.size() == 1);
        CHECK(vec[0] == 1);
    }

    SECTION("`std::string`")
    {
        std::string str;
        sys::meta::append_to(str, 'a');
        sys::meta::append_to(str, "bc");
        CHECK(str == "abc");
    }

    SECTION("Custom Types")
    {
        append_receiver catcher;
        sys::meta::append_to(catcher, 42); // NOLINT(readability-magic-numbers)
        CHECK(catcher.val == 42);
    }
}

// NOLINTEND(misc-include-cleaner)
