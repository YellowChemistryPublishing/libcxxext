import os
import sys
from typing import List

if len(sys.path) < 2 or not sys.path[1].endswith(".."):
    sys.path.insert(1, os.path.join(sys.path[0], ".."))

import lib.config as config
from lib.exec import (
    exec_or_fail,
    exec_pkgmgr_cache_update,
    find_command,
    has_stamp,
    stamp_id,
)
from lib.log import lassert_unsupported_bconf


def install(host_platform: str, cl_name: str) -> None:
    if has_stamp("tool_clang_tidy"):
        return

    if host_platform == "linux":
        exec_pkgmgr_cache_update(host_platform)
        exec_or_fail(
            [
                "sudo",
                "apt-get",
                "-o",
                "DPkg::Lock::Timeout=60",
                "install",
                "-y",
                "clang-tidy-19",
            ]
        )
        exec_or_fail(["clang-tidy-19", "--version"])
    elif "msys" in host_platform:
        exec_pkgmgr_cache_update(host_platform)
        if cl_name == "clang":
            exec_or_fail(
                [
                    "pacman",
                    "-S",
                    "--needed",
                    "--noconfirm",
                    f"mingw-w64-clang-x86_64-clang-tools-extra",
                ]
            )
        elif cl_name == "gcc":
            exec_or_fail(
                [
                    "pacman",
                    "-S",
                    "--needed",
                    "--noconfirm",
                    f"mingw-w64-x86_64-clang-tools-extra",
                ]
            )
        else:
            lassert_unsupported_bconf()
        exec_or_fail(["clang-tidy", "--version"])

    elif host_platform != "win32":
        lassert_unsupported_bconf()

    stamp_id("tool_clang_tidy")


def cmd() -> List[str]:
    return [
        find_command(
            ["clang-tidy"] + [f"clang-tidy-{ver}" for ver in range(25, 19, -1)]
        )
    ]


def run(args: List[str], *, host_platform: str, cl_name: str) -> None:
    if host_platform in config.support_platforms:
        install(host_platform, cl_name)
        exec_or_fail(cmd() + args)
    else:
        lassert_unsupported_bconf()
