#include <map>
#include <stack>
#include <string>
#include <utility>
#include <vector>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>

TEST_CASE("meta::generic_container_adaptor<...>::empty()", "[sys][meta][generic_container_adaptor]")
{
    {
        std::vector<int> cont;
        CHECK(sys::meta::generic_container_adaptor(cont).empty());

        cont.emplace_back(0);
        CHECK_FALSE(sys::meta::generic_container_adaptor(cont).empty());
        CHECK(cont.size() == 1uz);
        CHECK(cont.front() == 0);
    }

    {
        int cont[2] {};
        CHECK_FALSE(sys::meta::generic_container_adaptor(cont).empty());
    }
}
TEST_CASE("meta::generic_container_adaptor<...>::append_back(...)", "[sys][meta][generic_container_adaptor]")
{
    {
        std::vector<int> cont { 1, 2 };
        sys::meta::generic_container_adaptor(cont).append_back(3);

        CHECK(cont.size() == 3uz);
        CHECK(cont == std::vector<int> { 1, 2, 3 });
    }

    {
        std::string cont;
        sys::meta::generic_container_adaptor(cont).append_back('a');
        sys::meta::generic_container_adaptor(cont).append_back("de");
        sys::meta::generic_container_adaptor(cont).append_back(1uz, "bc");

        CHECK(cont == "abcde");
    }

    {
        struct
        {
            int got = 0; // NOLINT(misc-non-private-member-variables-in-classes)

            void emplace(int val) { this->got = val; }
        } cont;
        sys::meta::generic_container_adaptor(cont).append_back(1);

        CHECK(cont.got == 1);
    }

    {
        std::stack<int> cont;
        sys::meta::generic_container_adaptor(cont).append_back(1);

        CHECK(cont.size() == 1uz);
        CHECK(cont.top() == 1);
    }

    {
        std::map<int, int> cont;
        sys::meta::generic_container_adaptor(cont).append_back(std::make_pair(1, 2));

        CHECK(cont.size() == 1uz);
        CHECK(cont[1] == 2);
    }

    {
        struct
        {
            int got = 0; // NOLINT(misc-non-private-member-variables-in-classes)

            void push(int val) { this->got = val; }
        } cont;
        sys::meta::generic_container_adaptor(cont).append_back(1);

        CHECK(cont.got == 1);
    }

    {
        std::ostringstream cont;
        sys::meta::generic_container_adaptor(cont).append_back("yippppeeeeee ", "this works!!!");

        CHECK(cont.str() == "yippppeeeeee this works!!!");
    }
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
