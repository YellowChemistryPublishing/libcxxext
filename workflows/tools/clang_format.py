import urllib.request
from typing import List

from lib.exec import (
    exec_or_fail,
    find_command,
    has_stamp,
    stamp_id,
)
from lib.log import lassert_unsupported_bconf, lcheck_failed, lprint


def install(*, host_platform: str) -> None:
    if has_stamp("tool_clang_format"):
        return

    if host_platform != "linux":
        lassert_unsupported_bconf()

    urllib.request.urlretrieve(
        "https://apt.llvm.org/llvm.sh", "./tooling-build/llvm.sh"
    )
    exec_or_fail(["chmod", "+x", "./tooling-build/llvm.sh"])
    exec_or_fail(["sudo", "./tooling-build/llvm.sh", "19"])
    exec_or_fail(
        [
            "sudo",
            "apt-get",
            "-o",
            "DPkg::Lock::Timeout=60",
            "install",
            "-y",
            "clang-format-19",
        ]
    )

    clang_fmt_cmd = find_command(
        [f"clang-format-{ver}" for ver in range(25, 18, -1)] + ["clang-format"]
    )
    if not clang_fmt_cmd:
        lprint("Couldn't find `clang-format`.")
        lcheck_failed()

    exec_or_fail([clang_fmt_cmd, "--version"])
    stamp_id("tool_clang_format")


def cmd() -> List[str]:
    return [
        find_command(
            [f"clang-format-{ver}" for ver in range(25, 18, -1)] + ["clang-format"]
        )
    ]
