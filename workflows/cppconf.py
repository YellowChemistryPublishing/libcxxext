"""This is the primary workflow script for check:cppconf."""

from lib.exec import find_command

desc = (
    "Configure check for CMake projects."
    "@warning This check will `chdir` to the supplied src dir."
)

import argparse
import os
import shutil
import sys
from typing import List

import lib.config as config
import tools.cmake as cmake
import tools.cl as cl
import tools.clang_tidy as clang_tidy
from lib.exec import exec_or_fail, exec_pkgmgr_cache_update
from lib.log import lassert_unsupported_bconf, lcheck_passed


def additional_configure_flags(
    gen: str, target_arch: str, host_platform: str, cl_name: str
) -> List[str]:
    if cl_name == "msvc" and "Visual Studio" in gen:
        return [
            f"-G{gen}",
            "-A",
            "Win32" if target_arch == "i686" else "x64",
        ]
    elif cl_name == "clang" or cl_name == "gcc":
        return [
            f"-G{gen}",
            f"-DCMAKE_C_COMPILER={" ".join(cl.cmd_cc(host_platform=host_platform, cl_name=cl_name))}",
            f"-DCMAKE_CXX_COMPILER={" ".join(cl.cmd_cxx(host_platform=host_platform, cl_name=cl_name))}",
            "-DCMAKE_BUILD_TYPE=Debug",
        ] + (
            [
                "-DCMAKE_C_FLAGS=-m32",
                "-DCMAKE_CXX_FLAGS=-m32",
                "-DCMAKE_EXE_LINKER_FLAGS=-m32",
            ]
            if target_arch == "i686" and (cl_name == "clang" or cl_name == "gcc")
            else []
        )
    else:
        lassert_unsupported_bconf()

    raise AssertionError


def main(argv: List[str]) -> None:
    config.check_name = "check:cppconf"

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
        "-r",
        "--rimraf",
        default=False,
        help="Delete build dir before building.",
        action="store_true",
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

    if args.platform == "linux":
        if args.use_generator == "Ninja":
            exec_or_fail(
                [
                    "sudo",
                    "apt-get",
                    "-o",
                    "DPkg::Lock::Timeout=60",
                    "install",
                    "-y",
                    "ninja-build",
                ]
            )
            exec_or_fail(["ninja", "--version"])

        elif args.use_generator != "Unix Makefiles":
            lassert_unsupported_bconf()

    elif "msys" in args.platform:
        exec_pkgmgr_cache_update(args.platform)
        pacman_cmd = ["pacman", "-S", "--needed", "--noconfirm"]

        if args.use_generator == "Ninja":
            exec_or_fail(pacman_cmd + ["ninja"])
            exec_or_fail(["ninja", "--version"])

        elif args.use_generator == "MinGW Makefiles":
            if args.compiler == "clang":
                exec_or_fail(pacman_cmd + [f"mingw-w64-clang-{args.arch}-make"])
            elif args.compiler == "gcc":
                exec_or_fail(pacman_cmd + [f"mingw-w64-{args.arch}-make"])
            else:
                lassert_unsupported_bconf()

            exec_or_fail(["mingw32-make", "--version"])

        else:
            lassert_unsupported_bconf()

    elif args.platform != "win32":
        lassert_unsupported_bconf()

    cl.install(
        target_arch=args.arch, host_platform=args.platform, cl_name=args.compiler
    )
    clang_tidy.install(host_platform=args.platform, cl_name=args.compiler)

    if args.rimraf and os.path.exists(args.build_dir_name):
        shutil.rmtree(args.build_dir_name)

    cmake.run(
        [
            "-B",
            args.build_dir_name,
        ]
        + additional_configure_flags(
            args.use_generator, args.arch, args.platform, args.compiler
        )
        + (["--trace-expand", "--log-level=VERBOSE"] if args.cmake_verbose else []),
        host_platform=args.platform,
        cl_name=args.compiler,
    )

    lcheck_passed()


if __name__ == "__main__":
    main(sys.argv)
