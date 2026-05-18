// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>

_nowarn_begin_one_clang(_clwarn_clang_unused_function);
static int example_function_iif(int, float) { return 0; }
static void example_function_v() { }
_nowarn_end_clang();

namespace
{
    struct example_type
    {
        // NOLINTBEGIN(readability-convert-member-functions-to-static)
        [[nodiscard]] int mem_fn(float, double) { return 0; }
        [[nodiscard]] int cmem_fn(float) const { return 0; }
        // NOLINTEND(readability-convert-member-functions-to-static)

        static int static_fn(int) { return 0; }

        void c_fn() const { }
        void v_fn() volatile { }
        void cv_fn() const volatile { }

        void cl_fn() const& { }
        void vl_fn() volatile& { }
        void cvl_fn() const volatile& { }

        void cr_fn() const&& { }
        void vr_fn() volatile&& { }
        void cvr_fn() const volatile&& { }
    };
} // namespace

TEST_CASE("meta::function_signature<...>::return_type", "[sys][meta][function_signature]")
{
    STATIC_CHECK(std::same_as<sys::meta::function_signature<int(float, double)>::return_type, int>);
    STATIC_CHECK(std::same_as<sys::meta::function_signature<void()>::return_type, void>);
    STATIC_CHECK(std::same_as<sys::meta::function_signature<int (*)(float, double)>::return_type, int>);

    STATIC_CHECK(std::same_as<sys::meta::function_signature<int (example_type::*)(float, double)>::return_type, int>);
    STATIC_CHECK(std::same_as<sys::meta::function_signature<decltype(&example_type::mem_fn)>::return_type, int>);
}

TEST_CASE("meta::function_signature<...>::arguments", "[sys][meta][function_signature]")
{
    STATIC_CHECK(std::same_as<sys::meta::function_signature<int(float, double, char)>::arguments, std::tuple<float, double, char>>);
    STATIC_CHECK(std::same_as<sys::meta::function_signature<void()>::arguments, std::tuple<>>);
    STATIC_CHECK(std::same_as<sys::meta::function_signature<int (*)(float, double)>::arguments, std::tuple<float, double>>);
    STATIC_CHECK(std::same_as<sys::meta::function_signature<int (example_type::*)(float, double)>::arguments, std::tuple<float, double>>);
}

TEST_CASE("meta::function_signature<...>::is_member_func()", "[sys][meta][function_signature]")
{
    STATIC_CHECK(sys::meta::function_signature<int (example_type::*)(float, double)>::is_member_func());
    STATIC_CHECK(sys::meta::function_signature<decltype(&example_type::mem_fn)>::is_member_func());

    STATIC_CHECK_FALSE(sys::meta::function_signature<int(float)>::is_member_func());
    STATIC_CHECK_FALSE(sys::meta::function_signature<int (*)(float)>::is_member_func());
}

TEST_CASE("meta::function_signature<...>::is_signature_of<...>()", "[sys][meta][function_signature]")
{
    STATIC_CHECK(sys::meta::function_signature<int(int, float)>::is_signature_of<int (*)(int, float)>());
    STATIC_CHECK_FALSE(sys::meta::function_signature<int(int, float)>::is_signature_of<void (*)()>());
    STATIC_CHECK_FALSE(sys::meta::function_signature<int(int, float)>::is_signature_of<int (*)(int)>());
    STATIC_CHECK_FALSE(sys::meta::function_signature<int>::is_signature_of<void (*)()>());
    STATIC_CHECK_FALSE(sys::meta::function_signature<example_type>::is_signature_of<int (*)(int)>());
}

TEST_CASE("meta::function_signature<...>::is_const(), meta::function_signature<...>::is_volatile(), meta::function_signature<...>::is_lvalue(), "
          "meta::function_signature<...>::is_rvalue()",
          "[sys][meta][function_signature]")
{
    STATIC_CHECK(sys::meta::function_signature<decltype(&example_type::c_fn)>::is_const());
    STATIC_CHECK(sys::meta::function_signature<decltype(&example_type::v_fn)>::is_volatile());
    STATIC_CHECK(sys::meta::function_signature<decltype(&example_type::cv_fn)>::is_const());
    STATIC_CHECK(sys::meta::function_signature<decltype(&example_type::cv_fn)>::is_volatile());

    STATIC_CHECK(sys::meta::function_signature<decltype(&example_type::cl_fn)>::is_const());
    STATIC_CHECK(sys::meta::function_signature<decltype(&example_type::cl_fn)>::is_lvalue());
    STATIC_CHECK_FALSE(sys::meta::function_signature<decltype(&example_type::cl_fn)>::is_rvalue());

    STATIC_CHECK(sys::meta::function_signature<decltype(&example_type::cr_fn)>::is_const());
    STATIC_CHECK_FALSE(sys::meta::function_signature<decltype(&example_type::cr_fn)>::is_lvalue());
    STATIC_CHECK(sys::meta::function_signature<decltype(&example_type::cr_fn)>::is_rvalue());
}

TEST_CASE("IFunc<...>", "[sys][IFunc]")
{
    STATIC_CHECK(sys::IFunc<decltype(&example_function_iif), int(int, float)>);
    STATIC_CHECK(sys::IFunc<decltype(&example_function_v), void()>);

    STATIC_CHECK_FALSE(sys::IFunc<decltype(&example_function_iif), void()>);
    STATIC_CHECK_FALSE(sys::IFunc<decltype(&example_function_iif), int(int)>);

    STATIC_CHECK(sys::IFunc<std::function<int(int, float)>, int(int, float)>);
    STATIC_CHECK(sys::IFunc<decltype([](int x) -> int { return x; }), int(int)>);

    struct example_functor
    {
        int operator()(int, float) const { return -1; }
    };
    STATIC_CHECK(sys::IFunc<example_functor, int(int, float)>);
    STATIC_CHECK_FALSE(sys::IFunc<example_functor, void()>);
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
