import urllib.request
from typing import List

import lib.config as config
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

    llvm_sh_url = "https://apt.llvm.org/llvm.sh"
    llvm_sh_relp = f"./{config.tools_reldir}/llvm.sh"
    urllib.request.urlretrieve(llvm_sh_url, llvm_sh_relp)
    exec_or_fail(["chmod", "+x", llvm_sh_relp])
    exec_or_fail(["sudo", llvm_sh_relp, "19"])
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
