import os
import sys
import re
from textwrap import dedent
from typing import Dict, Set

sys.path.append(os.path.abspath(f"{os.path.dirname(__file__)}/.."))

import lib.config as config
from lib.log import lcheck_failed, lcheck_passed, lprint
from lib.uni import fetch_url, write_header

script_dir: str = os.path.dirname(os.path.abspath(__file__))
output_path: str = os.path.abspath(f"{script_dir}/../../sys/data/ClWarnGCC.h")

GCC_DOC_URL = "https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html"


def snakeify_gnu_warn_flag(flag: str) -> str:
    ret = flag
    if ret.startswith("-W"):
        ret = ret[2:]

    ret = ret.replace("++", "xx")
    ret = re.sub(r"[^a-zA-Z0-9]+", "_", ret)
    ret = re.sub(r"_+", "_", ret)
    return ret.strip("_").lower()


def parse_warnings(html: str) -> Set[str]:
    pattern = r"\>\s*(-W(?!no-)[a-zA-Z][-a-zA-Z0-9\+]+)\s*(\<|=)"
    matches = re.findall(pattern, html)

    ret: Set[str] = set()
    for warn, _ in matches:
        ret.add(warn)

    return ret


def main() -> None:
    config.check_name = "scripts:generate_gcc_warnings"

    html = fetch_url(GCC_DOC_URL)
    if not html:
        lprint("Failed to fetch GCC documentation.")
        lcheck_failed()

    warnings = parse_warnings(html)
    lprint(f"Found {len(warnings)} GCC warning options.")

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
            f.write(f'#define _clwarn_gcc_{mname} "{warn}"\n')

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
