import os
import sys
from typing import List
from textwrap import dedent

sys.path.append(os.path.abspath(f"{os.path.dirname(__file__)}/.."))

import lib.config as config
from lib.log import lcheck_passed, lprint
from lib.uni import to_cxx_u32_literal, write_header, parse_codepoints

proplist_path = os.path.abspath(
    f"{os.path.dirname(os.path.abspath(__file__))}/../../sys.Text/data_unicode/ucd/PropList.txt"
)
output_path = os.path.abspath(
    f"{os.path.dirname(os.path.abspath(__file__))}/../../sys.Text/data/UnicodeWhitespace.h"
)


def main(argv: List[str]) -> None:
    config.check_name = "scripts:generate_unicode_whitespace"

    whitespace_codepoints = parse_codepoints(proplist_path, "White_Space")

    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, "w", encoding="utf-8") as f:
        write_header(f, os.path.basename(output_path), __file__)

        f.write(
            dedent(
                """\
                #pragma once

                // NOLINTBEGIN(bugprone-branch-clone)

                namespace sys::internal
                {
                    /// @brief Check if a character is a Unicode whitespace character.
                    constexpr bool dchar_is_ws(const char32_t c) noexcept
                    {
                        switch (c)
                        {
                """
            )
        )
        for cp in sorted(whitespace_codepoints):
            cp_hex = hex(cp)[2:].upper()
            f.write(f"        case {to_cxx_u32_literal(cp_hex)}: return true;\n")
        f.write(
            dedent(
                """\
                        default: return false;
                        }
                    }
                } // namespace sys::internal

                // NOLINTEND(bugprone-branch-clone)
                """
            )
        )

    lprint(f"Successfully generated `{output_path}`.")
    lcheck_passed()


if __name__ == "__main__":
    main(sys.argv)
