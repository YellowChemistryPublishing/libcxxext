import os
import sys
from typing import List

import lib.config as config
from lib.exec import exec_or_fail, has_stamp, stamp_id
from lib.log import lassert_unsupported_bconf


def install(*, host_platform: str) -> None:
    if has_stamp("tool_mypy"):
        return

    if host_platform not in config.support_platforms:
        lassert_unsupported_bconf()

    venv_dir = f"./{config.tools_reldir}"
    python_cmd = (
        f"{venv_dir}/Scripts/python.exe"
        if host_platform == "win32"
        else f"{venv_dir}/bin/python"
    )

    if not os.path.exists(python_cmd):
        exec_or_fail([sys.executable, "-m", "venv", venv_dir])

    exec_or_fail([python_cmd, "-m", "pip", "install", "mypy"])

    mypy_cmd = (
        f"{venv_dir}/Scripts/mypy.exe"
        if host_platform == "win32"
        else f"{venv_dir}/bin/mypy"
    )
    exec_or_fail([mypy_cmd, "--version"])

    stamp_id("tool_mypy")


def cmd(*, host_platform: str) -> List[str]:
    venv_dir = f"./{config.tools_reldir}"
    return [
        (
            f"{venv_dir}/Scripts/mypy.exe"
            if host_platform == "win32"
            else f"{venv_dir}/bin/mypy"
        )
    ]


def run(args: List[str], *, host_platform: str) -> None:
    if host_platform in config.support_platforms:
        install(host_platform=host_platform)
        exec_or_fail(cmd(host_platform=host_platform) + args)
    else:
        lassert_unsupported_bconf()
