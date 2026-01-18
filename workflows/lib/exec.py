"""Execution utilities for workflows."""

import os
import re
import subprocess
import time
from typing import Callable, List

import lib.config as config
from lib.log import format_cmd_for_printing, lcheck_failed, log_invoc_failed, lprint


def str_key_replace(string: str, d: dict[str, str]) -> str:
    """Replace all keys in a string with their corresponding values."""

    for key, value in d.items():
        string = re.sub(rf"(?<!\\)\$\{{{re.escape(key)}\}}", value, string)
    return string


def find_command(candidates: List[str]) -> str:
    """Find the first available command from a list of candidates."""

    for cmd in candidates:
        try:
            subprocess.run([cmd], capture_output=True, timeout=5)
            return cmd
        except subprocess.TimeoutExpired:
            return cmd
        except FileNotFoundError:
            pass
    return None


def stamp_id(id: str) -> None:
    """Record a stamp with the given ID."""

    cache_stamp_path: str = f"{config.tools_reldir}/{id}_stamp"
    cache_update_interval_secs: int = 6 * 60 * 60  # 6 hours.

    if not os.path.exists(cache_stamp_path) or (
        time.time() - os.path.getmtime(cache_stamp_path) > cache_update_interval_secs
    ):
        os.makedirs(os.path.dirname(cache_stamp_path), exist_ok=True)
        with open(cache_stamp_path, "w", encoding="utf-8") as f:
            f.write(f"{time.ctime()}\n")


def has_stamp(id: str) -> bool:
    """Check if an in-date stamp with the given ID exists."""

    cache_stamp_path: str = f"{config.tools_reldir}/{id}_stamp"
    return os.path.exists(cache_stamp_path) and (
        time.time() - os.path.getmtime(cache_stamp_path) <= 6 * 60 * 60  # 6 hours.
    )


def mark_finding_ok(finding_file_name: str) -> None:
    """Flag a finding as ok."""

    stamp_id(f"findings_pass_{os.path.basename(finding_file_name)}")


def is_finding_ok(finding_file_name: str) -> bool:
    """Check if a finding is ok."""

    return has_stamp(f"findings_pass_{os.path.basename(finding_file_name)}")


def exec_or_fail(
    cmd: List[str], on_fail: Callable[[], None] = lambda: lcheck_failed(upstack=3)
) -> None:
    """Execute a command and exit if it fails."""

    if config.is_verbose:
        lprint(
            f"Running command `{format_cmd_for_printing(cmd)}`.",
            upstack=2,
        )
    result: subprocess.CompletedProcess[bytes] = subprocess.run(
        cmd, capture_output=config.is_silent
    )
    if result.returncode != 0:
        log_invoc_failed(result.stdout, result.stderr)
        lprint(
            f"Command `{format_cmd_for_printing(cmd)}` failed with exit code {result.returncode}.",
            upstack=2,
        )
        on_fail()


def exec_pkgmgr_cache_update(platform: str) -> None:
    """Update package manager cache, for platforms with one, if not done recently."""

    if not has_stamp("pkgmgr_cache_update"):
        stamp_id("pkgmgr_cache_update")

        if platform == "linux":
            exec_or_fail(["sudo", "apt-get", "-o", "DPkg::Lock::Timeout=60", "update"])
        elif "msys" in platform:
            exec_or_fail(["pacman", "-Syu", "--noconfirm"])
        else:
            lprint(
                f"No package manager for platform `{platform}`, skipping.",
                upstack=2,
            )
            return

    else:
        lprint(
            f"Skipping package manager cache update--ran recently. (Delete {config.tools_reldir}/pkgmgr_cache_update_stamp to force an update.)"
        )
