import os
import sys
from typing import List

import lib.config as config
from lib.exec import exec_or_fail, exec_pkgmgr_cache_update, has_stamp, stamp_id
from lib.log import lassert_unsupported_bconf


def install(*, host_platform: str) -> None:
    if has_stamp("tool_doxygen"):
        return

    if host_platform == "linux":
        exec_pkgmgr_cache_update(host_platform)
        apt_cmd = ["sudo", "apt-get", "-o", "DPkg::Lock::Timeout=60"]
        exec_or_fail(apt_cmd + ["install", "-y", "doxygen"])
    else:
        lassert_unsupported_bconf()

    exec_or_fail(["doxygen", "--version"])
    stamp_id("tool_doxygen")


def cmd() -> List[str]:
    return ["doxygen"]


def run(args: List[str], *, host_platform: str) -> None:
    if host_platform in config.support_platforms:
        install(host_platform=host_platform)
        exec_or_fail(cmd() + args)
    else:
        lassert_unsupported_bconf()
