"""This is the primary workflow script for check:cpppack."""

desc = "Packaging check for CMake projects."

import argparse
import glob
import os
import shutil
import sys
from pathlib import Path
from typing import List

import lib.config as config
import tools.cpack as cpack
from lib.log import lcheck_passed


KNOWN_ARCHIVE_EXTS: List[str] = [
    ".tar.gz",
    ".tar.bz2",
    ".tar.xz",
    ".tar.Z",
    ".tar.7z",
    ".sh",
    ".zip",
    ".deb",
    ".rpm",
    ".msi",
    ".exe",
    ".dmg",
    ".pkg",
    ".7z",
]


def main(argv: List[str]) -> None:
    config.check_name = "check:cpppack"

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
        choices=config.supported_compilers,
        metavar="",
        required=True,
    )
    parser.add_argument(
        "-c", "--config", help="CMake config name.", metavar="", required=True
    )
    parser.add_argument(
        "-d", "--src-dir", help="Full src dir path.", metavar="", required=True
    )
    parser.add_argument(
        "-b", "--build-dir-name", help="Build dir name.", metavar="", required=True
    )
    parser.add_argument(
        "-a",
        "--artifact-name",
        help=f"Artifact name when moved to `{config.artifacts_reldir}`.",
        metavar="",
        required=True,
    )
    parser.add_argument(
        "-i",
        "--package-artifacts",
        help="Which artifacts to package.",
        choices=["install", "src", "all"],
        metavar="",
        required=False,
        default="all",
    )
    parser.add_argument(
        "-cv",
        "--cmake-verbose",
        help="Make CPack output unreadable.",
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

    os.chdir(f"{args.src_dir}/{args.build_dir_name}")

    if args.package_artifacts in ["install", "all"]:
        cpack.run(
            [
                "--config",
                "CPackConfig.cmake",
                "-C",
                args.config,
            ]
            + (["--verbose"] if args.cmake_verbose else []),
            host_platform=args.platform,
            cl_name=args.compiler,
        )

    if args.package_artifacts in ["src", "all"]:
        cpack.run(
            [
                "--config",
                "CPackSourceConfig.cmake",
                "-C",
                args.config,
            ]
            + (["--verbose"] if args.cmake_verbose else []),
            host_platform=args.platform,
            cl_name=args.compiler,
        )

    artifacts_dir: str = f"../{config.artifacts_reldir}"
    os.makedirs(artifacts_dir, exist_ok=True)

    for entry in glob.glob("_CPack_Packages/*/*/*"):
        if os.path.isfile(entry):
            name: str = os.path.basename(entry)

            ext: str = ""
            for e in sorted(KNOWN_ARCHIVE_EXTS, key=len, reverse=True):
                if name.endswith(e):
                    ext = e
                    break
            else:
                ext = Path(name).suffix

            stem: str = name[: -len(ext)] if ext else name
            is_source: bool = stem.endswith("-Source")

            new_stem: str = args.artifact_name
            if is_source:
                new_stem += "-src"

            shutil.move(entry, f"{artifacts_dir}/{new_stem}{ext}")

    lcheck_passed()


if __name__ == "__main__":
    main(sys.argv)
