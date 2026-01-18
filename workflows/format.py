"""This is the primary workflow script for check:format."""

desc = "`clang-format` check for C++ projects."

import argparse
import difflib
import io
import os
import pathlib
import re
import subprocess
import sys
from typing import List

import lib.config as config
import tools.clang_format as clang_format
from lib.exec import exec_or_fail, mark_finding_ok
from lib.log import lcheck_failed, lcheck_passed, log_invoc_failed, lprint


def main(argv: List[str]) -> None:
    config.check_name = "check:format"

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
        "-f",
        "--regex-path-filter",
        help="Filter for files to analyze.",
        metavar="",
        required=False,
        default=".*\\.(h|hpp|c|cc|cpp|cxx|c++)$",
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

    clang_format.install(args.platform)

    pathlib.Path(f"{config.findings_reldir}/ClangFormat.md").unlink(missing_ok=True)

    unformatted_count: int = 0
    regex = re.compile(args.regex_path_filter)
    for root, _, files in os.walk("."):
        for file in files:
            file_relpath: str = os.path.relpath(os.path.join(root, file), ".").replace(
                "\\", "/"
            )
            if regex.match(file_relpath):
                clang_fmt_result: subprocess.CompletedProcess[bytes] = subprocess.run(
                    clang_format.cmd() + [file_relpath], capture_output=True
                )
                if clang_fmt_result.returncode != 0:
                    log_invoc_failed(clang_fmt_result.stdout, clang_fmt_result.stderr)
                    lcheck_failed()

                unformatted: str = (
                    io.open(file_relpath, "rb")
                    .read()
                    .decode(encoding="utf-8", errors="ignore")
                    .replace("\r", "\\r")
                )
                formatted: str = clang_fmt_result.stdout.decode(
                    encoding="utf-8", errors="ignore"
                ).replace("\r", "")

                diff: str = "".join(
                    difflib.unified_diff(
                        unformatted.splitlines(keepends=True),
                        formatted.splitlines(keepends=True),
                        fromfile="received",
                        tofile="expected",
                    )
                )
                if len(diff) > 0:
                    with open(f"{config.findings_reldir}/ClangFormat.md", "a") as f:
                        f.write(f"### `{file_relpath}`\n\n")
                        f.write("```diff\n")
                        f.write(diff)
                        f.write("```\n\n")
                    lprint(f"Found `{file_relpath}` was unformatted.")
                    unformatted_count += 1

    if unformatted_count > 0:
        lprint(f"Found {unformatted_count} unformatted files.")
    else:
        with open(f"{config.findings_reldir}/ClangFormat.md", "a") as f:
            pass
        lprint("Found no unformatted files.")
        mark_finding_ok(f"{config.findings_reldir}/ClangFormat.md")

    lcheck_passed()


if __name__ == "__main__":
    main(sys.argv)
