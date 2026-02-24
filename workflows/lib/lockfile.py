import os
import sys
import time
from io import BufferedRandom
from typing import Dict

import lib.config as config


tools_dir = f"{os.path.dirname(__file__)}/../../{config.tools_reldir}"

if sys.platform == "win32":
    import msvcrt as msvcrt

    def try_lock(f: BufferedRandom) -> bool:
        f.seek(0)
        f.write(b"\x00")
        f.flush()
        try:
            f.seek(0)
            msvcrt.locking(f.fileno(), msvcrt.LK_NBLCK, 1)
            return True
        except OSError:
            return False

    def unlock(f: BufferedRandom) -> None:
        f.seek(0)
        msvcrt.locking(f.fileno(), msvcrt.LK_UNLCK, 1)

else:
    import fcntl as fcntl

    def try_lock(f: BufferedRandom) -> bool:
        try:
            fcntl.flock(f.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
            return True
        except BlockingIOError:
            return False

    def unlock(f: BufferedRandom) -> None:
        fcntl.flock(f.fileno(), fcntl.LOCK_UN)


held: Dict[str, BufferedRandom] = {}


def lockfile_acq(name: str, timeout: int = 120) -> None:
    """Acquire a project-level lock."""

    path = f"{tools_dir}/{name}.lck"
    os.makedirs(os.path.dirname(path), exist_ok=True)

    start_time = time.time()
    while True:
        f = open(path, "a+b")
        if try_lock(f):
            f.seek(0)
            f.truncate()
            f.write(str(os.getpid()).encode())
            f.flush()
            held[name] = f
            return
        f.close()

        if time.time() - start_time > timeout:
            raise TimeoutError(
                f"Timeout acquiring project lock `{name}` after {timeout}s."
            )

        time.sleep(1)


def lockfile_rel(name: str) -> None:
    """Release a project-level lock."""

    if f := held.pop(name, None):
        unlock(f)
        path = f.name
        f.close()
        try:
            os.remove(path)
        except OSError:
            pass


def lockfile_relall() -> None:
    """Release all project-level locks."""

    for name in list(held):
        lockfile_rel(name)
