"""This is the primary workflow script for check:cppcov."""

desc = "Coverage reporter and report builder check for ctest'd CMake projects that produce gcov-style coverage data."

import argparse
import os
import sys
from typing import List

import lib.config as config
import tools.gcovr as gcovr
from lib.exec import exec_or_fail, mark_finding_ok
from lib.log import lcheck_passed


def main(argv: List[str]) -> None:
    config.check_name = "check:cppcov"

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
        choices=["clang", "gcc"],
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
        "-f",
        "--regex-path-filter",
        help="Filter for files to analyze.",
        metavar="",
        required=False,
        default=".*",
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

    os.makedirs(f"{args.build_dir_name}/coverage", exist_ok=True)
    gcovr.run(
        [
            "--filter",
            args.regex_path_filter,
            "--html",
            "--html-details",
            "-o",
            f"{args.build_dir_name}/coverage/coverage_report.html",
            args.build_dir_name,
        ],
        host_platform=args.platform,
        cl_name=args.compiler,
    )

    md_report_path = f"{config.findings_reldir}/CoverageReport.md"
    md_report_cmd = gcovr.cmd(cl_name=args.compiler) + [
        "--filter",
        args.regex_path_filter,
        "--markdown",
        "--gcov-object-directory",
        args.build_dir_name,
    ]

    content, _ = exec_or_fail(md_report_cmd, capture_output=True)
    content = content.replace("# GCC Code Coverage Report\n\n", "").replace("##", "###")
    content = (
        f"Detailed coverage report uploaded at `{args.build_dir_name}/coverage/coverage_report.html`.\n\n"
        + content
    )
    with open(md_report_path, "wb") as f:
        f.write(content.encode(encoding="utf-8", errors="ignore"))
    mark_finding_ok(md_report_path)

    lcheck_passed()


if __name__ == "__main__":
    main(sys.argv)
