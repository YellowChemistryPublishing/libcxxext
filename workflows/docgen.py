"""This is the primary workflow script for check:docgen."""

desc = "Documentation generator check for projects using Doxygen."

import argparse
import os
import sys
from typing import List

import lib.config as config
import tools.doxygen as doxygen
from lib.exec import exec_or_fail, mark_finding_ok
from lib.log import lcheck_failed, lcheck_passed


def main(argv: List[str]) -> None:
    config.check_name = "check:docgen"

    parser: argparse.ArgumentParser = argparse.ArgumentParser(
        prog=config.check_name,
        usage=f"python3 {os.path.basename(__file__)} [args]...",
        description=desc,
        epilog="We don't know what we're doing™.",
    )
    parser.add_argument(
        "-p",
        "--platform",
        help="Host platform.",
        choices=config.support_platforms,
        metavar="",
        required=True,
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

    doxygen.install(host_platform=args.platform)

    failed: bool = False

    def on_fail() -> None:
        nonlocal failed
        failed = True

    content, _ = exec_or_fail(doxygen.cmd(), capture_output=True, on_fail=on_fail)
    log_path = f"{config.findings_reldir}/Docgen.log"
    with open(log_path, "w") as f:
        f.write(content)
    if failed:
        lcheck_failed()

    mark_finding_ok(log_path)
    lcheck_passed()


if __name__ == "__main__":
    main(sys.argv)
