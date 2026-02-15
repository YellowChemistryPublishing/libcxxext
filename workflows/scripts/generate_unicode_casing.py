import os
import sys
from textwrap import dedent, indent
from typing import List, Dict, Set

sys.path.append(os.path.abspath(f"{os.path.dirname(__file__)}/.."))

import lib.config as config
from lib.log import lcheck_passed, lprint
from lib.uni import to_cxx_u32_literal, write_header, parse_codepoints

script_dir: str = os.path.dirname(os.path.abspath(__file__))
ucd_dir: str = os.path.abspath(f"{script_dir}/../../sys.Text/data_unicode/ucd")
unicode_data_path: str = os.path.abspath(f"{ucd_dir}/UnicodeData.txt")
special_casing_path: str = os.path.abspath(f"{ucd_dir}/SpecialCasing.txt")
derived_core_props_path: str = os.path.abspath(f"{ucd_dir}/DerivedCoreProperties.txt")
proplist_path: str = os.path.abspath(f"{ucd_dir}/PropList.txt")

output_path: str = os.path.abspath(f"{script_dir}/../../sys.Text/data/UnicodeCasing.h")


# C++ casing context expression equivalents.
CONDITION_MAP = {
    "Final_Sigma": "ctx.is_final_sigma",
    "After_Soft_Dotted": "ctx.after_soft_dotted",
    "More_Above": "ctx.more_above",
    "After_I": "ctx.after_i",
    "Before_Dot": "ctx.before_dot",
    "Not_Before_Dot": "!ctx.before_dot",
}


def should_add_to_conditional(
    cp: int,
    cps: List[int],
    simple_mappings: Dict[int, int],
    unconditional: Dict[int, List[int]],
) -> bool:
    """Determine if a codepoint should be added to conditional casing."""

    if len(cps) > 1:
        return True
    if cp in simple_mappings and simple_mappings[cp] != cps[0]:
        return True
    uncond = unconditional.get(cp)
    if uncond is not None and uncond != cps:
        return True
    return False


def main(argv: List[str]) -> None:
    config.check_name = "scripts:generate_unicode_casing"

    # Aggregate 1:1 (simple) mappings.
    simple_lower_mappings: Dict[int, int] = {}
    simple_upper_mappings: Dict[int, int] = {}
    with open(unicode_data_path, "r", encoding="utf-8") as f:
        for line in f:
            parts: List[str] = line.split(";")
            if len(parts) < 14:
                continue
            cp: int = int(parts[0], 16)
            upper_hex: str = parts[12].strip()
            if upper_hex:
                simple_upper_mappings[cp] = int(upper_hex, 16)
            lower_hex: str = parts[13].strip()
            if lower_hex:
                simple_lower_mappings[cp] = int(lower_hex, 16)

    # Parse codepoints with special properties.
    cased_points: Set[int] = parse_codepoints(derived_core_props_path, "Cased")
    case_ignorable_points: Set[int] = parse_codepoints(
        derived_core_props_path, "Case_Ignorable"
    )
    soft_dotted_points: Set[int] = parse_codepoints(proplist_path, "Soft_Dotted")

    # Aggregate special casing rules.
    unconditional_special_lower: Dict[int, List[int]] = {}
    unconditional_special_upper: Dict[int, List[int]] = {}
    conditional_special_lower: List[Dict] = []
    conditional_special_upper: List[Dict] = []
    with open(special_casing_path, "r", encoding="utf-8") as f:
        for line in f:
            clean_line: str = line.split("#")[0].strip()
            if not clean_line:
                continue
            sub_parts: List[str] = [p.strip() for p in clean_line.split(";")]
            if len(sub_parts) < 4:
                continue

            codepoint: int = int(sub_parts[0], 16)
            lower_cps: List[int] = [int(x, 16) for x in sub_parts[1].split()]
            upper_cps: List[int] = [int(x, 16) for x in sub_parts[3].split()]

            if len(sub_parts) == 4 or not sub_parts[4]:
                if len(lower_cps) > 1 or (
                    codepoint in simple_lower_mappings
                    and simple_lower_mappings[codepoint] != lower_cps[0]
                ):
                    unconditional_special_lower[codepoint] = lower_cps

                if len(upper_cps) > 1 or (
                    codepoint in simple_upper_mappings
                    and simple_upper_mappings[codepoint] != upper_cps[0]
                ):
                    unconditional_special_upper[codepoint] = upper_cps

            else:
                condition_parts: List[str] = sub_parts[4].split()
                language: str = ""
                condition: str = ""
                for p in condition_parts:
                    if p.lower() in ("tr", "az", "lt"):
                        language = p.lower()
                    else:
                        condition = p

                entry_lower = {
                    "cp": codepoint,
                    "lower": lower_cps,
                    "condition": condition,
                    "language": language,
                }
                entry_upper = {
                    "cp": codepoint,
                    "upper": upper_cps,
                    "condition": condition,
                    "language": language,
                }

                if should_add_to_conditional(
                    codepoint,
                    lower_cps,
                    simple_lower_mappings,
                    unconditional_special_lower,
                ):
                    conditional_special_lower.append(entry_lower)
                if should_add_to_conditional(
                    codepoint,
                    upper_cps,
                    simple_upper_mappings,
                    unconditional_special_upper,
                ):
                    conditional_special_upper.append(entry_upper)

    # Generate C++ Header
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, "w", encoding="utf-8") as f:
        write_header(f, os.path.basename(output_path), __file__)

        f.write(
            dedent(
                """\
                #pragma once

                #include <string_view>

                #include <LanguageSupport.h>

                namespace sys
                {
                    /// @brief Context for special casing rules.
                    struct casing_context
                    {
                        bool is_preceded_by_cased : 1 = false;
                        bool is_final_sigma : 1 = false;
                        bool after_soft_dotted : 1 = false;
                        bool more_above : 1 = false;
                        bool after_i : 1 = false;
                        bool before_dot : 1 = false;
                    };
                } // namespace sys

                // NOLINTBEGIN(readability-function-size, bugprone-branch-clone, cppcoreguidelines-pro-bounds-pointer-arithmetic, hicpp-multiway-paths-covered)

                namespace sys::internal
                {
                """
            )
        )

        def write_bool_switch(name: str, points_set: Set[int]) -> None:
            f.write(
                indent(
                    dedent(
                        f"""\
                        constexpr bool {name}(const char32_t c) noexcept
                        {{
                            switch (c)
                            {{
                        """
                    ),
                    "    ",
                )
            )
            for cp in sorted(list(points_set)):
                cp_lit: str = to_cxx_u32_literal(hex(cp)[2:].upper())
                f.write(f"        case {cp_lit}: return true;\n")
            f.write(
                indent(
                    dedent(
                        """\
                            default: return false;
                            }
                        }
                        """
                    ),
                    "    ",
                )
            )

        write_bool_switch("dchar_is_cased", cased_points)
        write_bool_switch("dchar_is_case_ignorable", case_ignorable_points)
        write_bool_switch("dchar_is_soft_dotted", soft_dotted_points)
        f.write("\n")

        def write_simple_mapping(
            name: str,
            direction: str,
            mappings: Dict[int, int],
            unconditional_special: Dict[int, List[int]],
        ) -> None:
            f.write(
                indent(
                    dedent(
                        f"""\
                        /// @brief Simple (1:1, unconditional) {direction}case mapping.
                        constexpr char32_t {name}(const char32_t c) noexcept
                        {{
                            switch (c)
                            {{
                        """
                    ),
                    "    ",
                )
            )
            for cp, mapped_cp in sorted(mappings.items()):
                if (
                    cp in unconditional_special
                    and len(unconditional_special[cp]) == 1
                    and unconditional_special[cp][0] == mapped_cp
                ):
                    continue
                cp_lit: str = to_cxx_u32_literal(hex(cp)[2:].upper())
                mapped_lit: str = to_cxx_u32_literal(hex(mapped_cp)[2:].upper())
                f.write(f"        case {cp_lit}: return {mapped_lit};\n")
            f.write(
                indent(
                    dedent(
                        """\
                            default: return c;
                            }
                        }
                        """
                    ),
                    "    ",
                )
            )

        write_simple_mapping(
            "dchar_to_lower_simple",
            "lower",
            simple_lower_mappings,
            unconditional_special_lower,
        )
        write_simple_mapping(
            "dchar_to_upper_simple",
            "upper",
            simple_upper_mappings,
            unconditional_special_upper,
        )
        f.write("\n")

        def write_special_mapping(
            direction: str,
            entries: List[Dict],
            unconditional: Dict[int, List[int]],
            simple_func: str,
        ) -> None:
            f.write(
                indent(
                    dedent(
                        f"""\
                        /// @brief Special (conditional, 1:N) {direction}case mapping.
                        constexpr sz dchar_to_{direction}_special(char32_t out[], const char32_t c, unsafe, const std::u8string_view lang = u8"",
                                                                  [[maybe_unused]] const sys::casing_context& ctx = sys::casing_context()) noexcept
                        {{
                        """
                    ),
                    "    ",
                )
            )

            for entry in entries:
                cond: str = entry["condition"]
                cond_expr: str = CONDITION_MAP.get(cond, "true")

                lang_expr: str = "true"
                if entry["language"]:
                    lang_expr = f"lang == u8\"{entry['language']}\""

                cp_lit: str = to_cxx_u32_literal(hex(entry["cp"])[2:].upper())
                mapping = entry[direction]
                assignments = "\n                                ".join(
                    [
                        f"out[{idx}] = {to_cxx_u32_literal(hex(mapped_cp)[2:].upper())};"
                        for idx, mapped_cp in enumerate(mapping)
                    ]
                )

                f.write(
                    indent(
                        dedent(
                            f"""\
                            if (c == {cp_lit}{f" && {lang_expr}" if lang_expr != "true" else ""}{f" && {cond_expr}" if cond_expr != "true" else ""})
                            {{
                                {assignments}
                                return {len(mapping)}_uz;
                            }}
                            """
                        ),
                        "        ",
                    )
                )

            f.write(
                indent(
                    dedent(
                        """\
                            switch (c)
                            {
                        """
                    ),
                    "        ",
                )
            )
            for cp, cp_list in sorted(unconditional.items()):
                ret_cp: str = to_cxx_u32_literal(hex(cp)[2:].upper())
                assignments = "\n                                ".join(
                    [
                        f"out[{idx}] = {to_cxx_u32_literal(hex(mapped_cp)[2:].upper())};"
                        for idx, mapped_cp in enumerate(cp_list)
                    ]
                )
                f.write(
                    indent(
                        dedent(
                            f"""\
                            case {ret_cp}:
                                {assignments}
                                return {len(cp_list)}_uz;
                            """
                        ),
                        "        ",
                    )
                )
            f.write(
                indent(
                    dedent(
                        f"""\
                            default:
                                out[0] = {simple_func}(c);
                                return 1_uz;
                            }}
                        }}
                        """
                    ),
                    "    ",
                )
            )

        write_special_mapping(
            "lower",
            conditional_special_lower,
            unconditional_special_lower,
            "dchar_to_lower_simple",
        )
        write_special_mapping(
            "upper",
            conditional_special_upper,
            unconditional_special_upper,
            "dchar_to_upper_simple",
        )

        f.write(
            dedent(
                """\
                } // namespace sys::internal

                // NOLINTEND(readability-function-size, bugprone-branch-clone, cppcoreguidelines-pro-bounds-pointer-arithmetic, hicpp-multiway-paths-covered)
                """
            )
        )

    lprint(f"Successfully generated `{output_path}`.")
    lcheck_passed()


if __name__ == "__main__":
    main(sys.argv)
