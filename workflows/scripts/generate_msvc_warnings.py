import os
import sys
import re
from textwrap import dedent
from typing import Dict, List, Tuple

sys.path.append(os.path.abspath(f"{os.path.dirname(__file__)}/.."))

import lib.config as config
from lib.log import lcheck_passed, lprint
from lib.uni import fetch_url, write_header

script_dir: str = os.path.dirname(os.path.abspath(__file__))
output_path: str = os.path.abspath(f"{script_dir}/../../sys/data/ClWarnMSVC.h")

PORTAL_URL = "https://learn.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warnings-c4000-c5999"
BASE_URL = "https://learn.microsoft.com/en-us/cpp/error-messages/compiler-warnings"


def parse_portal(html: str) -> List[str]:
    pattern = r'href="(compiler-warnings-c[0-9]+-through-c[0-9]+\?view=msvc-170)"'
    links = re.findall(pattern, html)
    return [BASE_URL + "/" + link for link in links]


def clean_description(desc: str) -> str:
    desc = re.sub(r"<[^>]+>", "", desc)  # Strip HTML tags.
    desc = re.sub(r"[^a-zA-Z0-9]+", "_", desc)  # Replace any non-alphanumeric with '_'.
    desc = re.sub(r"_+", "_", desc)  # Collapse sequential underscores.
    return desc.strip("_").lower()


def parse_warning_page(html: str) -> List[Tuple[str, str]]:
    # Match <td> content generically.
    row_pattern = r"<tr>\s*<td>\s*(.*?)\s*</td>\s*<td>\s*(.*?)\s*</td>\s*</tr>"
    rows = re.findall(row_pattern, html, re.DOTALL)

    ret: List[Tuple[str, str]] = []
    for id_col, desc_col in rows:
        # Extract ID `Cxxxx(?x)` from the first column.
        id_match = re.search(r"\b(C[0-9]{4,5})\b", id_col)
        if id_match:
            warn_id = id_match.group(1)[1:]
            description = clean_description(desc_col)
            if description:
                ret.append((warn_id, description))
            else:
                # Fallback to ID if no description found.
                ret.append((warn_id, f"warning_{warn_id}"))

    return ret


def main() -> None:
    config.check_name = "scripts:generate_msvc_warnings"

    sub_pages = parse_portal(fetch_url(PORTAL_URL))
    if not sub_pages:
        lprint("No sub-pages found on portal. Using a hard-coded list.")
        sub_pages = [
            f"{BASE_URL}/compiler-warnings-c4000-through-c4199",
            f"{BASE_URL}/compiler-warnings-c4200-through-c4399",
            f"{BASE_URL}/compiler-warnings-c4400-through-c4599",
            f"{BASE_URL}/compiler-warnings-c4600-through-c4799",
            f"{BASE_URL}/compiler-warnings-c4800-through-c4999",
            f"{BASE_URL}/compiler-warnings-c5000-through-c5199",
            f"{BASE_URL}/compiler-warnings-c5200-through-c5399",
        ]

    warning_data: Dict[str, str] = {}
    for page in sub_pages:
        warnings = parse_warning_page(fetch_url(page))
        for warn_id, desc in warnings:
            if warn_id not in warning_data:
                warning_data[warn_id] = desc
        lprint(f"Parsed {len(warnings)} warnings from `{page}`.")
    lprint(f"Found {len(warning_data)} unique warnings.")

    desc_to_ids: Dict[str, List[str]] = {}
    for wid, desc in warning_data.items():
        if desc not in desc_to_ids:
            desc_to_ids[desc] = []
        desc_to_ids[desc].append(wid)

    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, "w", encoding="utf-8") as f:
        f.write("#pragma once\n\n")
        write_header(f, os.path.basename(output_path), __file__)
        f.write(
            dedent(
                """\
                // NOLINTBEGIN(modernize-macro-to-enum)

                /// @addtogroup compiler_warnings
                /// @{

                """
            )
        )

        macro_kvs: List[Tuple[str, str]] = []
        for wid, desc in warning_data.items():
            if len(desc_to_ids[desc]) > 1:
                # Append ID on collision.
                macro_name = f"_clwarn_msvc_{desc}_{wid}"
            else:
                macro_name = f"_clwarn_msvc_{desc}"

            if len(macro_name) > 120:
                macro_name = f"_clwarn_msvc_{wid}"

            macro_kvs.append((wid, macro_name))

        for wid, mname in sorted(macro_kvs):
            f.write(f"#define {mname} {wid}\n")

        f.write(
            dedent(
                """\

                /// @}

                // NOLINTEND(modernize-macro-to-enum)
                """
            )
        )

    lprint(f"Successfully generated `{output_path}`.")
    lcheck_passed()


if __name__ == "__main__":
    main()
