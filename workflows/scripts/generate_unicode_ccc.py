import os
import sys
from textwrap import dedent
from typing import List, Dict

sys.path.append(os.path.abspath(f"{os.path.dirname(__file__)}/.."))

import lib.config as config
from lib.log import lcheck_passed, lprint
from lib.uni import to_cxx_u32_literal, write_header

script_dir: str = os.path.dirname(os.path.abspath(__file__))
ucd_dir: str = os.path.abspath(f"{script_dir}/../../sys.Text/data_unicode/ucd")
property_value_aliases_path: str = os.path.abspath(
    f"{ucd_dir}/PropertyValueAliases.txt"
)
unicode_data_path: str = os.path.abspath(f"{ucd_dir}/UnicodeData.txt")

output_path: str = os.path.abspath(f"{script_dir}/../../sys.Text/data/UnicodeCCC.h")


def parse_ccc_names(path: str) -> Dict[int, str]:
    """Parse Canonical Combining Class (CCC) names from data file at `property_value_aliases_path`."""

    names: Dict[int, str] = {}
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.split("#")[0].strip()
            if not line:
                continue

            parts: List[str] = [p.strip() for p in line.split(";")]
            if parts[0] == "ccc":
                # ccc; <val>; <short_name>; <long_name>
                try:
                    val: int = int(parts[1])
                    if val not in names and len(parts) >= 4:
                        name: str = parts[3].lower()
                        names[val] = name
                except ValueError:
                    continue

    return names


def main(argv: List[str]) -> None:
    config.check_name = "scripts:generate_unicode_ccc"

    ccc_names: Dict[int, str] = parse_ccc_names(property_value_aliases_path)
    ccc_map: Dict[int, int] = {}

    with open(unicode_data_path, "r", encoding="utf-8") as f:
        for line in f:
            parts: List[str] = line.split(";")
            if len(parts) < 4:
                continue
            cp: int = int(parts[0], 16)
            ccc: int = int(parts[3])
            if ccc != 0:
                ccc_map[cp] = ccc

    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, "w", encoding="utf-8") as f:
        write_header(f, os.path.basename(output_path), __file__)

        f.write(
            dedent(
                """\
                #pragma once

                #include <cstdint>

                namespace sys
                {
                    /// @brief Unicode Canonical Combining Class (CCC).
                    enum class canonical_combining_class : uint_least8_t
                    {
                """
            )
        )
        for val in range(0, 256):
            name = ccc_names.get(val, f"ccc{val}")
            f.write(f"        {name} = {val}u,\n")
        f.write(
            dedent(
                """\
                    };
                } // namespace sys

                // NOLINTBEGIN(bugprone-branch-clone, readability-function-size)

                namespace sys::internal
                {
                    /// @brief Get the Canonical Combining Class (CCC) for a codepoint.
                    constexpr sys::canonical_combining_class dchar_ccc(const char32_t c) noexcept
                    {
                        switch (c)
                        {
                """
            )
        )
        for cp, ccc in sorted(ccc_map.items()):
            cp_lit: str = to_cxx_u32_literal(hex(cp)[2:].upper())
            name = ccc_names.get(ccc, f"ccc{ccc}")
            f.write(
                f"        case {cp_lit}: return sys::canonical_combining_class::{name};\n"
            )
        f.write(
            dedent(
                """\
                        default: return sys::canonical_combining_class::not_reordered;
                        }
                    }
                } // namespace sys::internal

                // NOLINTEND(bugprone-branch-clone, readability-function-size)
                """
            )
        )

    lprint(f"Successfully generated `{output_path}`.")
    lcheck_passed()


if __name__ == "__main__":
    main(sys.argv)
