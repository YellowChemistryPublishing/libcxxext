"""This is the primary workflow script for job:findings."""

desc = (
    "Report findings after checksets finish."
    "@warning This check will `chdir` to the supplied workdir."
)

import argparse
import os
import sys
from pathlib import Path
from typing import List

import lib.config as config
from lib.exec import is_finding_ok


def main(argv: List[str]) -> None:
    config.check_name = "job:findings"

    parser: argparse.ArgumentParser = argparse.ArgumentParser(
        prog=config.check_name,
        usage=f"python3 {os.path.basename(__file__)} [args]...",
        description=desc,
        epilog="We don't know what we're doing™.",
    )
    parser.add_argument(
        "-d", "--workdir", help="Working directory.", metavar="", required=True
    )
    parser.add_argument(
        "-v",
        "--verbose",
        help="Extra logging messages to stdout.",
        action="store_true",
    )
    parser.add_argument(
        "-s",
        "--suppress",
        help="Silence tool invocation output except on failure.",
        action="store_true",
    )
    args: argparse.Namespace = parser.parse_args()

    config.is_verbose = args.verbose
    config.is_silent = args.suppress

    os.chdir(args.workdir)

    has_failing_finding: bool = False
    for filename in os.listdir(config.findings_reldir):
        filepath = f"{config.findings_reldir}/{filename}"
        if os.path.isfile(filepath):
            content: str = ""
            with open(filepath, "r") as f:
                content = f.read()

            print(f"## {Path(filepath).stem}\n")
            if not is_finding_ok(filepath):
                print("> [!CAUTION]  \n> Finding did not report success.\n")
                has_failing_finding = True

            if content.strip() == "":
                print("Check found no findings.\n")

            else:
                print("<details>\n")
                print("<summary><b>View Findings</b></summary>\n")

                if Path(filepath).suffix == ".md":
                    print(content)
                else:
                    print("```")
                    print(content.strip())
                    print("```\n")

                print("</details>\n")

    if has_failing_finding:
        exit(1)

    # Unfortunately, this would show up in the findings too, so we just skip it.
    # lcheck_passed()


if __name__ == "__main__":
    main(sys.argv)
