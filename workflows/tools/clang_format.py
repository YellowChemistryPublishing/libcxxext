import os
import sys
import urllib.request
from typing import List

if len(sys.path) < 2 or not sys.path[1].endswith(".."):
    sys.path.insert(1, os.path.join(sys.path[0], ".."))

import tools.cl as cl
from lib.exec import (
    exec_or_fail,
    find_command,
    has_stamp,
    stamp_id,
)
from lib.log import lcheck_failed, lprint


def install(host_platform: str) -> None:
    if has_stamp("tool_clang_format"):
        return

    urllib.request.urlretrieve(
        "https://apt.llvm.org/llvm.sh", "./tooling-build/llvm.sh"
    )
    exec_or_fail(["chmod", "+x", "./tooling-build/llvm.sh"])
    exec_or_fail(["sudo", "./tooling-build/llvm.sh", "21"])
    exec_or_fail(
        [
            "sudo",
            "apt-get",
            "-o",
            "DPkg::Lock::Timeout=60",
            "install",
            "-y",
            "clang-format-21",
        ]
    )

    clang_fmt_cmd = find_command(
        [f"clang-format-{ver}" for ver in range(25, 19, -1)] + ["clang-format"]
    )
    if not clang_fmt_cmd:
        lprint("Couldn't find `clang-format`.")
        lcheck_failed()

    exec_or_fail([clang_fmt_cmd, "--version"])
    stamp_id("tool_clang_format")


def cmd() -> List[str]:
    return [
        find_command(
            [f"clang-format-{ver}" for ver in range(25, 19, -1)] + ["clang-format"]
        )
    ]
