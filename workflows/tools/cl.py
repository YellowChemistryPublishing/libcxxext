from lib.log import lcheck_failed
import os
import sys
import urllib.request
import time
from typing import List

if len(sys.path) < 2 or not sys.path[1].endswith(".."):
    sys.path.insert(1, os.path.join(sys.path[0], ".."))

from lib.exec import exec_or_fail, exec_pkgmgr_cache_update, has_stamp, stamp_id
from lib.log import lassert_unsupported_bconf, lcheck_failed


def install(target_arch: str, host_platform: str, cl_name: str) -> None:
    if has_stamp("tool_cl"):
        return

    if host_platform == "linux":
        exec_pkgmgr_cache_update(host_platform)

        if cl_name == "clang":
            urllib.request.urlretrieve(
                "https://apt.llvm.org/llvm.sh", "./tooling-build/llvm.sh"
            )
            exec_or_fail(["chmod", "+x", "./tooling-build/llvm.sh"])

            # Locks apt repository, try a few times in case another process is using it.
            for i in range(10):

                def on_fail():
                    if i == 9:
                        lcheck_failed()

                exec_or_fail(["sudo", "./tooling-build/llvm.sh", "21"], on_fail=on_fail)
                time.sleep(1)

            exec_or_fail(
                [
                    "sudo",
                    "apt-get",
                    "-o",
                    "DPkg::Lock::Timeout=60",
                    "install",
                    "-y",
                    "clang-21",
                ]
            )

            exec_or_fail(["clang-21", "--version"])
            exec_or_fail(["clang++-21", "--version"])

            if target_arch == "i686":
                exec_or_fail(
                    [
                        "sudo",
                        "apt-get",
                        "install",
                        "-y",
                        "libc6-dev-i386",
                        "libstdc++-13-dev",
                        "gcc-13-multilib",
                        "g++-13-multilib",
                    ]
                )

                exec_or_fail(["gcc-13", "--version"])
                exec_or_fail(["g++-13", "--version"])

        elif cl_name == "gcc":
            exec_or_fail(
                [
                    "sudo",
                    "apt-get",
                    "-o",
                    "DPkg::Lock::Timeout=60",
                    "install",
                    "-y",
                    "gcc-13-multilib",
                    "g++-13-multilib",
                ]
            )

            exec_or_fail(["gcc-13", "--version"])
            exec_or_fail(["g++-13", "--version"])

        else:
            lassert_unsupported_bconf()

    elif "msys" in host_platform:
        exec_pkgmgr_cache_update(host_platform)

        if cl_name == "clang":
            exec_or_fail(
                [
                    "pacman",
                    "-S",
                    "--needed",
                    "--noconfirm",
                    "mingw-w64-clang-x86_64-clang",
                ]
            )
            exec_or_fail(["clang", "--version"])
            exec_or_fail(["clang++", "--version"])

        elif cl_name == "gcc":
            exec_or_fail(
                [
                    "pacman",
                    "-S",
                    "--needed",
                    "--noconfirm",
                    "mingw-w64-x86_64-gcc",
                ]
            )
            exec_or_fail(["x86_64-w64-mingw32-gcc", "-v"])
            exec_or_fail(["x86_64-w64-mingw32-g++", "-v"])

        else:
            lassert_unsupported_bconf()

    elif host_platform != "win32":
        lassert_unsupported_bconf()

    stamp_id("tool_cl")


def cmd_cc(host_platform: str, cl_name: str) -> List[str]:
    if host_platform == "linux":
        if cl_name == "clang":
            return ["clang-21"]
        elif cl_name == "gcc":
            return ["gcc-13"]
    elif "msys" in host_platform:
        if cl_name == "clang":
            return ["clang"]
        elif cl_name == "gcc":
            return ["x86_64-w64-mingw32-gcc"]
    else:
        lassert_unsupported_bconf()


def cmd_cxx(host_platform: str, cl_name: str) -> List[str]:
    if host_platform == "linux":
        if cl_name == "clang":
            return ["clang++-21"]
        elif cl_name == "gcc":
            return ["g++-13"]
    elif "msys" in host_platform:
        if cl_name == "clang":
            return ["clang++"]
        elif cl_name == "gcc":
            return ["x86_64-w64-mingw32-g++"]
    else:
        lassert_unsupported_bconf()
