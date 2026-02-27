import os
import re
import sys
from textwrap import dedent
from typing import Dict, Set

sys.path.append(os.path.abspath(f"{os.path.dirname(__file__)}/.."))

import lib.config as config
from lib.log import lcheck_failed, lcheck_passed, lprint
from lib.uni import fetch_url, write_header
from scripts.generate_gcc_warnings import snakeify_gnu_warn_flag

script_dir: str = os.path.dirname(os.path.abspath(__file__))
output_path: str = os.path.abspath(f"{script_dir}/../../sys/data/ClWarnClang.h")

CLANG_DOC_URL = "https://clang.llvm.org/docs/DiagnosticsReference.html"


def parse_warnings(html: str) -> Set[str]:
    nav_match = re.search(r'<nav[^>]*id="contents".*?</nav>', html, re.DOTALL)
    if not nav_match:
        return set()

    nav_html = nav_match.group(0)
    pattern = r"\>\s*(-W[a-zA-Z#][-a-zA-Z0-9\+]+)\s*(\<)"
    matches = re.findall(pattern, nav_html)

    ret: Set[str] = set()
    for warn, _ in matches:
        ret.add(warn)

    return ret


def main() -> None:
    config.check_name = "scripts:generate_clang_warnings"

    html = fetch_url(CLANG_DOC_URL)
    if not html:
        lprint("Failed to fetch Clang documentation.")
        lcheck_failed()

    warnings = parse_warnings(html)
    lprint(f"Found {len(warnings)} Clang warning options.")

    sorted_warnings = sorted(list(warnings))

    macro_to_warns: Dict[str, str] = {}
    for warn in sorted_warnings:
        wm = snakeify_gnu_warn_flag(warn)
        if wm not in macro_to_warns:
            macro_to_warns[wm] = warn
        else:
            lprint(f"Found duplicate warning {warn} using macro {wm}, stopping.")
            lcheck_failed()

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

        for mname in sorted(macro_to_warns.keys()):
            warn = macro_to_warns[mname]
            f.write(f'#define _clwarn_clang_{mname} "{warn}"\n')

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
