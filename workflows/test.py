"""This is the primary workflow script for check:test."""

desc = (
    "Test check for CMake projects."
    "@warning This check will `chdir` to the supplied src dir."
)

import argparse
import os
import sys
from typing import List

import lib.config as config
import tools.ctest as ctest
from lib.log import lcheck_passed


def main(argv: List[str]) -> None:
    config.check_name = "check:test"

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

    ctest.run(
        [
            "--test-dir",
            str(args.build_dir_name),
            "--output-on-failure",
        ]
        + (["--debug", "--extra-verbose"] if args.cmake_verbose else []),
        host_platform=args.platform,
        cl_name=args.compiler,
    )

    lcheck_passed()


if __name__ == "__main__":
    main(sys.argv)
