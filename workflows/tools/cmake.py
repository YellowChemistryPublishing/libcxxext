import os
import sys
from typing import List

from lib.exec import Lockfile

if len(sys.path) < 2 or not sys.path[1].endswith(".."):
    sys.path.insert(1, os.path.join(sys.path[0], ".."))

import lib.config as config
from lib.exec import exec_or_fail, exec_pkgmgr_cache_update, has_stamp, stamp_id
from lib.log import lassert_unsupported_bconf


def install(*, host_platform: str, cl_name: str) -> None:
    if has_stamp("tool_cmake"):
        return

    exec_pkgmgr_cache_update(host_platform)

    with Lockfile("pkgmgr"):
        if host_platform == "linux":
            apt_cmd = ["sudo", "apt-get"]
            exec_or_fail(apt_cmd + ["install", "-y", "cmake"])
        elif "msys" in host_platform:
            pacman_cmd = ["pacman", "-S", "--needed", "--noconfirm"]
            if cl_name == "clang":
                exec_or_fail(pacman_cmd + ["mingw-w64-clang-x86_64-cmake"])
            elif cl_name == "gcc":
                exec_or_fail(pacman_cmd + ["mingw-w64-x86_64-cmake"])

    exec_or_fail(["cmake", "--version"])
    exec_or_fail(["ctest", "--version"])
    stamp_id("tool_cmake")


def cmd() -> List[str]:
    return ["cmake"]


def run(args: List[str], *, host_platform: str, cl_name: str) -> None:
    if host_platform in config.support_platforms:
        install(host_platform=host_platform, cl_name=cl_name)
        exec_or_fail(cmd() + args)
    else:
        lassert_unsupported_bconf()
