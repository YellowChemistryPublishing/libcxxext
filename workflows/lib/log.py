"""Logging utilities for workflows."""

import inspect
import os
import re
import threading
import time
from typing import List

import lib.config as config


shards: dict[int, int] = {}
next_shard_id: int = 0
shard_lock: threading.Lock = threading.Lock()


def format_cmd_for_printing(cmd: List[str]) -> str:
    """Format a command for printing."""

    return " ".join(
        [(f"'{c}'" if " " in c or "(" in c or ")" in c or "^" in c else c) for c in cmd]
    )


def lprint(string: str, upstack: int = 1) -> None:
    """Log a string with a newline."""

    with shard_lock:
        thread_id: int = threading.get_ident()
        if thread_id not in shards:
            global next_shard_id
            shards[thread_id] = next_shard_id
            next_shard_id += 1
        shard_id = shards[thread_id]

        caller_frame: inspect.FrameInfo = inspect.stack()[upstack]
        caller_filename = os.path.basename(caller_frame.filename)

        pstr: str = (
            f"\x1b[36;1m{f"[{caller_filename}:{caller_frame.lineno}]\x1b[0m ".ljust(24)}\x1b[38;5;208;1m{f"[shard-{"main" if shard_id == 0 else f"{shard_id}"}]\x1b[0m ".ljust(18)} {string}"
        )

        print(pstr)

        os.makedirs(config.findings_reldir, exist_ok=True)

        start_time = time.time()
        while time.time() - start_time < 4:
            try:
                with open(
                    config.lprint_templog_relp,
                    "a",
                    encoding="utf-8",
                ) as out:
                    out.write(re.sub(r"\x1b\[[0-9;]*m", "", pstr) + "\n")
                break
            except (IOError, OSError):
                time.sleep(0.1)


def log_invoc_failed(stdout: bytes, stderr: bytes) -> None:
    """Log tool invocation failure stdout and stderr."""

    lprint(
        f"Tool invocation failed with stdout:{" [empty]" if stdout is None else f"\n{stdout.decode(encoding="utf-8", errors="ignore")}"}",
        upstack=2,
    )
    lprint(
        f"... and stderr:{" [empty]" if stderr is None else f"\n{stderr.decode(encoding="utf-8", errors="ignore")}"}",
        upstack=2,
    )


def lcheck_passed(upstack: int = 2) -> None:
    """Log that a check has passed."""

    lprint(
        f"\x1b[32;3mPassed\x1b[0m \x1b[1;3m{config.check_name}.\x1b[0m",
        upstack=upstack,
    )

    exit(0)


def lcheck_failed(upstack: int = 2) -> None:
    """Log that a check has failed."""

    lprint(
        f"\x1b[31;3mFailed\x1b[0m \x1b[1;3m{config.check_name}.\x1b[0m",
        upstack=upstack,
    )

    exit(1)


def lassert_unsupported_bconf(upstack: int = 2) -> None:
    """Log that a build configuration is unsupported."""

    lprint("\x1b[31;3mUnsupported build configuration.\x1b[0m", upstack=upstack)
    lcheck_failed(upstack=upstack)
