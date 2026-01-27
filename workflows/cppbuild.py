"""This is the primary workflow script for check:cppbuild."""

desc = (
    "Build check for CMake projects."
    "@warning This check will `chdir` to the supplied src dir."
)

import argparse
import os
import sys
from typing import List

import lib.config as config
import tools.cmake as cmake
from lib.log import lassert_unsupported_bconf, lcheck_passed


def additional_build_flags(gen: str, cl_name: str) -> List[str]:
    if cl_name == "msvc" and "Visual Studio" in gen:
        return [
            "--target",
            "ALL_BUILD",
            "--config",
            "Debug",
            "--",
            "/p:CL_MPCount=8",
        ]
    elif cl_name == "clang" or cl_name == "gcc":
        return [
            "--target",
            "all",
        ]
    else:
        lassert_unsupported_bconf()

    raise AssertionError


def main(argv: List[str]) -> None:
    config.check_name = "check:cppbuild"

    parser: argparse.ArgumentParser = argparse.ArgumentParser(
        prog=config.check_name,
        usage=f"python3 {os.path.basename(__file__)} [args]...",
        description=desc,
        epilog="We don't know what we're doing™.",
    )
    parser.add_argument(
        "-m",
        "--arch",
        help="Target architecture.",
        choices=config.support_archs,
        metavar="",
        required=True,
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
        "-gen", "--use-generator", help="CMake generator name.", required=True
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

    cmake.run(
        [
            "--build",
            args.build_dir_name,
            "--parallel",
        ]
        + additional_build_flags(gen=args.use_generator, cl_name=args.compiler)
        + (["--verbose"] if args.cmake_verbose else []),
        host_platform=args.platform,
        cl_name=args.compiler,
    )

    lcheck_passed()


if __name__ == "__main__":
    main(sys.argv)
