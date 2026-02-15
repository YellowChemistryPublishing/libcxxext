import os
import sys
from textwrap import dedent, indent
from typing import Any, Dict, List, Set

sys.path.append(os.path.abspath(f"{os.path.dirname(__file__)}/.."))

import lib.config as config
from lib.log import lcheck_passed, lprint
from lib.uni import to_cxx_u32_literal, write_header, parse_codepoints

script_dir: str = os.path.dirname(os.path.abspath(__file__))
ucd_dir: str = os.path.abspath(f"{script_dir}/../../sys.Text/data_unicode/ucd")
unicode_data_path: str = os.path.abspath(f"{ucd_dir}/UnicodeData.txt")
special_casing_path: str = os.path.abspath(f"{ucd_dir}/SpecialCasing.txt")
case_folding_path: str = os.path.abspath(f"{ucd_dir}/CaseFolding.txt")
derived_core_props_path: str = os.path.abspath(f"{ucd_dir}/DerivedCoreProperties.txt")
proplist_path: str = os.path.abspath(f"{ucd_dir}/PropList.txt")

output_path: str = os.path.abspath(f"{script_dir}/../../sys.Text/data/UnicodeCasing.h")


# C++ casing context expression equivalents.
CONDITION_MAP = {
    "Final_Sigma": "fctx.is_preceded_by_cased && !lctx.followed_by_cased",
    "After_Soft_Dotted": "fctx.after_soft_dotted",
    "More_Above": "lctx.more_above",
    "After_I": "fctx.after_i",
    "Before_Dot": "lctx.before_dot",
    "Not_Before_Dot": "!lctx.before_dot",
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

    # Aggregate case folding mappings.
    simple_fold_mappings: Dict[int, int] = {}
    unconditional_special_fold: Dict[int, List[int]] = {}
    conditional_special_fold: List[Dict[str, Any]] = []
    with open(case_folding_path, "r", encoding="utf-8") as f:
        for line in f:
            cf_clean_line: str = line.split("#")[0].strip()
            if not cf_clean_line:
                continue

            cf_parts: List[str] = [p.strip() for p in cf_clean_line.split(";")]
            if len(cf_parts) < 3:
                continue

            cf_cp: int = int(cf_parts[0], 16)
            status: str = cf_parts[1]
            mapping: List[int] = [int(x, 16) for x in cf_parts[2].split()]

            if status in ("C", "S"):
                simple_fold_mappings[cf_cp] = mapping[0]
            if status in ("C", "F"):
                if len(mapping) > 1 or (
                    cf_cp in simple_fold_mappings
                    and simple_fold_mappings[cf_cp] != mapping[0]
                ):
                    unconditional_special_fold[cf_cp] = mapping
            elif status == "T":
                conditional_special_fold.append(
                    {"cp": cf_cp, "fold": mapping, "condition": "", "language": "tr"}
                )
                conditional_special_fold.append(
                    {"cp": cf_cp, "fold": mapping, "condition": "", "language": "az"}
                )

    # Parse codepoints with special properties.
    cased_points: Set[int] = parse_codepoints(derived_core_props_path, "Cased")
    case_ignorable_points: Set[int] = parse_codepoints(
        derived_core_props_path, "Case_Ignorable"
    )
    soft_dotted_points: Set[int] = parse_codepoints(proplist_path, "Soft_Dotted")

    # Aggregate special casing rules.
    unconditional_special_lower: Dict[int, List[int]] = {}
    unconditional_special_upper: Dict[int, List[int]] = {}
    conditional_special_lower: List[Dict[str, object]] = []
    conditional_special_upper: List[Dict[str, object]] = []
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
                #include <Numeric.h>

                namespace sys
                {
                    /// @brief Context for forward-looking casing rules.
                    struct forward_casing_context
                    {
                        bool is_preceded_by_cased : 1 = false;
                        bool after_soft_dotted : 1 = false;
                        bool after_i : 1 = false;
                    };
                    /// @brief Context for precomputed lookahead casing rules.
                    struct lookahead_casing_context
                    {
                        bool followed_by_cased : 1 = false;
                        bool more_above : 1 = false;
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
            mappings: Dict[int, int],
            unconditional_special: Dict[int, List[int]],
        ) -> None:
            f.write(
                indent(
                    dedent(
                        f"""\
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
            "dchar_to_lower_simple", simple_lower_mappings, unconditional_special_lower
        )
        write_simple_mapping(
            "dchar_to_upper_simple", simple_upper_mappings, unconditional_special_upper
        )
        write_simple_mapping(
            "dchar_fold_simple", simple_fold_mappings, unconditional_special_fold
        )
        f.write("\n")

        def write_special_mapping(
            name: str,
            entry_direction: str,
            entries: List[Dict[str, Any]],
            unconditional: Dict[int, List[int]],
            simple_func: str,
            is_fold: bool = False,
        ) -> None:
            context_params = (
                ""
                if is_fold
                else ", [[maybe_unused]] const sys::forward_casing_context& fctx, [[maybe_unused]] const sys::lookahead_casing_context& lctx"
            )
            f.write(
                indent(
                    dedent(
                        f"""\
                        constexpr sz {name}(char32_t out[], const char32_t c, const std::u8string_view lang{context_params}, unsafe) noexcept
                        {{
                        """
                    ),
                    "    ",
                )
            )

            for entry in entries:
                cond: str = entry.get("condition", "")
                cond_expr: str = CONDITION_MAP.get(cond, "true")

                lang_expr: str = "true"
                if entry["language"]:
                    lang_expr = f"lang == u8\"{entry['language']}\""

                cp_lit: str = to_cxx_u32_literal(hex(entry["cp"])[2:].upper())
                mapping = entry[entry_direction]
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
            "dchar_to_lower_special",
            "lower",
            conditional_special_lower,
            unconditional_special_lower,
            "dchar_to_lower_simple",
        )
        write_special_mapping(
            "dchar_to_upper_special",
            "upper",
            conditional_special_upper,
            unconditional_special_upper,
            "dchar_to_upper_simple",
        )
        write_special_mapping(
            "dchar_fold_special",
            "fold",
            conditional_special_fold,
            unconditional_special_fold,
            "dchar_fold_simple",
            is_fold=True,
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
