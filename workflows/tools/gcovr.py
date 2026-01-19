import os
import sys
import urllib.request
from typing import List

if len(sys.path) < 2 or not sys.path[1].endswith(".."):
    sys.path.insert(1, os.path.join(sys.path[0], ".."))

import lib.config as config
import tools.cl as cl
from lib.exec import exec_or_fail, find_command, has_stamp, stamp_id
from lib.log import lassert_unsupported_bconf, lcheck_failed, lprint


def install(host_platform: str) -> None:
    if has_stamp("tool_gcovr"):
        return

    lprint("Downloading gcovr...")
    if host_platform == "linux":
        try:
            os.makedirs(config.tools_reldir, exist_ok=True)
            urllib.request.urlretrieve(
                config.gcovr_url_linux,
                f"{config.tools_reldir}/gcovr",
            )
            os.chmod(f"{config.tools_reldir}/gcovr", 0o755)
        except Exception as e:
            lprint(f"Failed to download gcovr: {e}")
            lcheck_failed()
    elif "msys" in host_platform:
        try:
            os.makedirs(config.tools_reldir, exist_ok=True)
            urllib.request.urlretrieve(
                config.gcovr_url_msys,
                f"{config.tools_reldir}/gcovr",
            )
        except Exception as e:
            lprint(f"Failed to download gcovr: {e}")
            lcheck_failed()
    else:
        lassert_unsupported_bconf()
    lprint("Downloading gcovr... Done.")

    exec_or_fail([f"./{config.tools_reldir}/gcovr", "--version"])
    stamp_id("tool_gcovr")


def cmd(cl_name: str) -> List[str]:
    extra_args = []
    if cl_name == "clang" or cl_name == "gcc":
        extra_args += [
            "--gcov-executable",
            " ".join(cl.cmd_gcov(cl_name=cl_name)),
        ]
    else:
        lassert_unsupported_bconf()

    return [f"./{config.tools_reldir}/gcovr"] + extra_args


def run(args: List[str], *, host_platform: str, cl_name: str) -> None:
    if host_platform in config.support_platforms:
        install(host_platform=host_platform)
        exec_or_fail(cmd(cl_name=cl_name) + args)
    else:
        lassert_unsupported_bconf()
