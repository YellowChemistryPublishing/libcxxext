"""This is the primary workflow script for check:tidy."""

desc = (
    "Static analyzer check for CMake projects using `cmake/clang_tidy.cmake`."
    "@warning This check will `chdir` to the supplied src dir."
)

import argparse
import os
import subprocess
import sys
from typing import List

import lib.config as config
import tools.cmake as cmake
from lib.exec import mark_finding_ok
from lib.log import lcheck_failed, lcheck_passed, log_invoc_failed


def main(argv: List[str]) -> None:
    config.check_name = "check:tidy"

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
        "-cl",
        "--compiler",
        help="Compiler to use.",
        choices=config.support_compilers,
        metavar="",
        required=True,
    )
    parser.add_argument(
        "-d", "--src-dir", help="Full src dir path.", metavar="", required=True
    )
    parser.add_argument(
        "-b", "--build-dir-name", help="Build dir name.", metavar="", required=True
    )
    parser.add_argument(
        "-cv",
        "--cmake-verbose",
        help="Make CMake output unreadable.",
        action="store_true",
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

    os.chdir(args.src_dir)

    result: subprocess.CompletedProcess = subprocess.run(
        cmake.cmd()
        + [
            "--build",
            str(args.build_dir_name),
            "--parallel",
            "--target",
            "clang_tidy_all",
        ]
        + (["--verbose"] if args.cmake_verbose else []),
        capture_output=True,
    )
    if result.returncode != 0 and result.returncode != 1:
        log_invoc_failed(result.stdout, result.stderr)
        lcheck_failed()

    with open(f"{config.findings_reldir}/ClangTidy.log", "w") as f:
        f.write(result.stdout.decode(encoding="utf-8", errors="ignore"))

    mark_finding_ok(f"{config.findings_reldir}/ClangTidy.log")

    lcheck_passed()


if __name__ == "__main__":
    main(sys.argv)
