import os
import sys
from typing import List

if len(sys.path) < 2 or not sys.path[1].endswith(".."):
    sys.path.insert(1, os.path.join(sys.path[0], ".."))

import tools.cmake as cmake
import lib.config as config
from lib.exec import exec_or_fail
from lib.log import lassert_unsupported_bconf


def cmd() -> List[str]:
    return ["ctest"]


def run(args: List[str], *, host_platform: str, cl_name: str) -> None:
    if host_platform in config.support_platforms:
        cmake.install(host_platform=host_platform, cl_name=cl_name)
        exec_or_fail(cmd() + args)
    else:
        lassert_unsupported_bconf()
