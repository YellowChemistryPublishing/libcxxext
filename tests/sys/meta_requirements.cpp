#include <memory>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>

TEST_CASE("INullablePointer<...>", "[sys][INullablePointer]")
{
    STATIC_CHECK(sys::INullablePointer<void*>);
#if _libcxxext_compiler_msvc // Because this doesn't actually satisfy `sys::INothrowSwappable` on libstdc++.
    STATIC_CHECK(sys::INullablePointer<std::exception_ptr>);
#endif
    STATIC_CHECK(sys::INullablePointer<std::unique_ptr<int>::pointer>);
}

TEST_CASE("ITriviallyRelocatable<...>", "[sys][ITriviallyRelocatable]")
{
    struct example_relocatable_type : sys::traits::trivially_relocatable<example_relocatable_type>
    {
        example_relocatable_type() noexcept = default;
        example_relocatable_type(const example_relocatable_type&) noexcept = default;
        example_relocatable_type(example_relocatable_type&&) noexcept = default;
        ~example_relocatable_type() /* NOLINT(modernize-use-equals-default) */ { }

        example_relocatable_type& operator=(const example_relocatable_type&) noexcept = default;
        example_relocatable_type& operator=(example_relocatable_type&&) noexcept = default;
    };
    struct example_non_relocatable_type : example_relocatable_type
    {
        example_non_relocatable_type() noexcept = default;
        example_non_relocatable_type(const example_non_relocatable_type&) noexcept = default;
        example_non_relocatable_type(example_non_relocatable_type&&) noexcept = default;
        ~example_non_relocatable_type() /* NOLINT(modernize-use-equals-default) */ { }

        example_non_relocatable_type& operator=(const example_non_relocatable_type&) noexcept = default;
        example_non_relocatable_type& operator=(example_non_relocatable_type&&) noexcept = default;
    };

    STATIC_CHECK_FALSE(sys::ITriviallyRelocatable<void>);
    STATIC_CHECK(sys::ITriviallyRelocatable<unsigned long long>);
    STATIC_CHECK(sys::ITriviallyRelocatable<example_relocatable_type>);
    STATIC_CHECK_FALSE(sys::ITriviallyRelocatable<example_non_relocatable_type>);
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner)
