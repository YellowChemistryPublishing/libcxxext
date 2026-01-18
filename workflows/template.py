"""This is the primary workflow script for check:[CHECK NAME HERE]."""

desc = (
    "[WHAT DOES THIS CHECK DO] check for CMake projects."
    "[WHATEVER ELSE YOU WANT TO SAY]"
)

import argparse
import os
import sys
from typing import List

import lib.config as config
from lib.log import lcheck_passed


def main(argv: List[str]) -> None:
    config.check_name = "check:[CHECK NAME HERE]"

    parser: argparse.ArgumentParser = argparse.ArgumentParser(
        prog=config.check_name,
        usage=f"python3 {os.path.basename(__file__)} [args]...",
        description=desc,
        epilog="We don't know what we're doing™.",
    )
    # Add arguments here.
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

    # Execute your check here.

    lcheck_passed()


if __name__ == "__main__":
    main(sys.argv)
