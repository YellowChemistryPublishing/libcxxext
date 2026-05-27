#include <map>
#include <memory>
#include <ranges>
#include <string>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>
#include <module/sys.Alloc>
#include <module/sys.Containers>

template <typename TestType>
std::initializer_list<typename TestType::value_type>& wantIl4()
{
    if constexpr (std::same_as<typename TestType::value_type, i64>)
    {
        static std::initializer_list<i64> il { 1_i64, 2_i64, 3_i64, 4_i64 };
        return il;
    }
    else if constexpr (std::same_as<typename TestType::value_type, std::shared_ptr<int>>)
    {
        static std::initializer_list<std::shared_ptr<int>> il { std::make_shared<int>(1), std::make_shared<int>(2), std::make_shared<int>(3), nullptr };
        return il;
    }
    else if constexpr (std::same_as<typename TestType::value_type, std::string>)
    {
        static std::initializer_list<std::string> il { "one", "two", "three", "four strings!!" };
        return il;
    }
    else if constexpr (std::same_as<typename TestType::value_type, std::map<std::string, std::string>>)
    {
        static std::initializer_list<std::map<std::string, std::string>> il {
            { { "key1", "value1" } }, { { "key2", "value2" } }, { { "key3", "value3" } }, { { "key4", "value4" } }
        };
        return il;
    }
    else
    {
        std::initializer_list<typename TestType::value_type> emptyIl {};
        return emptyIl;
    }
};
template <typename TestType>
std::initializer_list<typename TestType::value_type>& wantIl8()
{
    if constexpr (std::same_as<typename TestType::value_type, i64>)
    {
        static std::initializer_list<i64> il { 1_i64, 2_i64, 3_i64, 4_i64, 5_i64, 6_i64, 7_i64, 8_i64 };
        return il;
    }
    else if constexpr (std::same_as<typename TestType::value_type, std::shared_ptr<int>>)
    {
        static std::initializer_list<std::shared_ptr<int>> il { std::make_shared<int>(1), std::make_shared<int>(2),
                                                                std::make_shared<int>(3), std::make_shared<int>(4),
                                                                std::make_shared<int>(5), nullptr,
                                                                std::make_shared<int>(7), nullptr };
        return il;
    }
    else if constexpr (std::same_as<typename TestType::value_type, std::string>)
    {
        static std::initializer_list<std::string> il { "one", "two", "three", "four", "five", "six", "seven", "eight stringies!!" };
        return il;
    }
    else if constexpr (std::same_as<typename TestType::value_type, std::map<std::string, std::string>>)
    {
        static std::initializer_list<std::map<std::string, std::string>> il { { { "key1", "value1" } }, { { "key2", "value2" } }, { { "key3", "value3" } },
                                                                              { { "key4", "value4" } }, { { "key5", "value5" } }, { { "key6", "value6" } },
                                                                              { { "key7", "value7" } }, { { "key8", "value8" } } };
        return il;
    }
    else
    {
        std::initializer_list<typename TestType::value_type> emptyIl {};
        return emptyIl;
    }
};

template <typename TestType>
typename TestType::value_type wantValue()
{
    if constexpr (std::same_as<typename TestType::value_type, i64>)
    {
        static i64 ret = 42_i64;
        return ret;
    }
    else if constexpr (std::same_as<typename TestType::value_type, std::shared_ptr<int>>)
    {
        static std::shared_ptr<int> ret = std::make_shared<int>(42);
        return ret;
    }
    else if constexpr (std::same_as<typename TestType::value_type, std::string>)
    {
        static std::string ret = "forty-two";
        return ret;
    }
    else
        return {};
};

TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ (
    "vec<...>(vec&&), operator=(const vec&), operator=(vec&&), vec<...>::ctor(const vec&), vec<...>::ctor(std::initializer_list<...>)", "[sys.Containers][vec]",
    (sys::vec<i64, sys::system_allocator<i64>>), (sys::vec<i64, sys::inplace_allocator<i64, 8uz>>), (sys::vec<i64, sys::small_buffer_allocator<i64, 4uz>>),
    (sys::vec<std::shared_ptr<int>, sys::system_allocator<std::shared_ptr<int>>>), (sys::vec<std::shared_ptr<int>, sys::inplace_allocator<std::shared_ptr<int>, 8uz>>),
    (sys::vec<std::shared_ptr<int>, sys::small_buffer_allocator<std::shared_ptr<int>, 4uz>>), (sys::vec<std::string, sys::system_allocator<std::string>>),
    (sys::vec<std::string, sys::inplace_allocator<std::string, 8uz>>), (sys::vec<std::string, sys::small_buffer_allocator<std::string, 4uz>>),
    (sys::vec<std::map<std::string, std::string>, sys::system_allocator<std::map<std::string, std::string>>>),
    (sys::vec<std::map<std::string, std::string>, sys::inplace_allocator<std::map<std::string, std::string>, 8uz>>),
    (sys::vec<std::map<std::string, std::string>, sys::small_buffer_allocator<std::map<std::string, std::string>, 4uz>>))
{
    {
        TestType vFrom = TestType(TestType::ctor(wantIl4<TestType>()).expect());
        TestType v = TestType::ctor(vFrom).expect();
        CHECK(std::ranges::equal(v.slice(), vFrom.slice()));
        CHECK(v.assign_from(vFrom));
        CHECK(std::ranges::equal(v.slice(), vFrom.slice()));
        CHECK(v.assign_from(std::move(vFrom)));

        CHECK_FALSE(v.empty());
        CHECK(v.size() == 4_uz);
        CHECK(v.capacity() >= v.size());
        CHECK(v.data());
        CHECK(std::ranges::equal(v.as_view(), v.slice()));
        CHECK(std::ranges::equal(v.slice(), wantIl4<TestType>()));
    }

    {
        TestType vFrom = TestType::ctor(wantIl8<TestType>()).expect();
        TestType v = TestType::ctor(vFrom).expect();
        CHECK(std::ranges::equal(v.slice(), vFrom.slice()));
        CHECK(v.assign_from(vFrom));
        CHECK(std::ranges::equal(v.slice(), vFrom.slice()));
        CHECK(v.assign_from(std::move(vFrom)));

        CHECK_FALSE(v.empty());
        CHECK(v.size() == 8_uz);
        CHECK(v.capacity() >= v.size());
        CHECK(v.data());
        CHECK(std::ranges::equal(v.as_view(), v.slice()));
        CHECK(std::ranges::equal(v.slice(), wantIl8<TestType>()));
    }
}

TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ (
    "vec<...>(vec&&), operator=(const vec&), operator=(vec&&), vec<...>::ctor(const vec&), vec<...>::ctor(sz, const T&)", "[sys.Containers][vec]",
    (sys::vec<i64, sys::system_allocator<i64>>), (sys::vec<i64, sys::inplace_allocator<i64, 8uz>>), (sys::vec<i64, sys::small_buffer_allocator<i64, 4uz>>),
    (sys::vec<std::shared_ptr<int>, sys::system_allocator<std::shared_ptr<int>>>), (sys::vec<std::shared_ptr<int>, sys::inplace_allocator<std::shared_ptr<int>, 8uz>>),
    (sys::vec<std::shared_ptr<int>, sys::small_buffer_allocator<std::shared_ptr<int>, 4uz>>), (sys::vec<std::string, sys::system_allocator<std::string>>),
    (sys::vec<std::string, sys::inplace_allocator<std::string, 8uz>>), (sys::vec<std::string, sys::small_buffer_allocator<std::string, 4uz>>),
    (sys::vec<std::map<std::string, std::string>, sys::system_allocator<std::map<std::string, std::string>>>),
    (sys::vec<std::map<std::string, std::string>, sys::inplace_allocator<std::map<std::string, std::string>, 8uz>>),
    (sys::vec<std::map<std::string, std::string>, sys::small_buffer_allocator<std::map<std::string, std::string>, 4uz>>))
{
    {
        TestType vFrom = TestType(TestType::ctor(4_uz, wantValue<TestType>()).expect());
        TestType v = TestType::ctor(vFrom).expect();
        CHECK(std::ranges::equal(v.slice(), vFrom.slice()));
        CHECK(v.assign_from(vFrom));
        CHECK(std::ranges::equal(v.slice(), vFrom.slice()));
        CHECK(v.assign_from(std::move(vFrom)));

        CHECK_FALSE(v.empty());
        CHECK(v.size() == 4_uz);
        CHECK(v.capacity() >= v.size());
        CHECK(v.data());
        CHECK(std::ranges::equal(v.as_view(), v.slice()));
        CHECK(std::ranges::equal(
            v.slice(), std::initializer_list<typename TestType::value_type> { wantValue<TestType>(), wantValue<TestType>(), wantValue<TestType>(), wantValue<TestType>() }));
    }

    {
        TestType vFrom = TestType(TestType::ctor(8_uz, wantValue<TestType>()).expect());
        TestType v = TestType::ctor(vFrom).expect();
        CHECK(std::ranges::equal(v.slice(), vFrom.slice()));
        CHECK(v.assign_from(vFrom));
        CHECK(std::ranges::equal(v.slice(), vFrom.slice()));
        CHECK(v.assign_from(std::move(vFrom)));

        CHECK_FALSE(v.empty());
        CHECK(v.size() == 8_uz);
        CHECK(v.capacity() >= v.size());
        CHECK(v.data());
        CHECK(std::ranges::equal(v.as_view(), v.slice()));
        CHECK(std::ranges::equal(v.slice(),
                                 std::initializer_list<typename TestType::value_type> { wantValue<TestType>(), wantValue<TestType>(), wantValue<TestType>(), wantValue<TestType>(),
                                                                                        wantValue<TestType>(), wantValue<TestType>(), wantValue<TestType>(),
                                                                                        wantValue<TestType>() }));
    }
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
