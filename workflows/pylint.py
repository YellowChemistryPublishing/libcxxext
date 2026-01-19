"""This is the primary workflow script for check:pylint."""

desc = "Static analyzer check for Python projects using `mypy`."

import argparse
import os
import sys
from typing import List

import lib.config as config
import tools.mypy as mypy
from lib.exec import exec_or_fail, mark_finding_ok
from lib.log import lcheck_passed


def main(argv: List[str]) -> None:
    config.check_name = "check:pylint"

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
        choices=["win32", "linux", "macos"],
        metavar="",
        required=True,
    )
    parser.add_argument(
        "-d", "--src-dir", help="Full src dir path.", metavar="", required=True
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

    mypy.install(host_platform=args.platform)

    content, _ = exec_or_fail(
        mypy.cmd(host_platform=args.platform) + ["--strict", args.src_dir],
        capture_output=True,
    )
    os.makedirs(config.findings_reldir, exist_ok=True)
    log_path = f"{config.findings_reldir}/PythonLint.log"
    with open(log_path, "w", encoding="utf-8") as f:
        f.write(content)
    mark_finding_ok(log_path)

    lcheck_passed()


if __name__ == "__main__":
    main(sys.argv)
