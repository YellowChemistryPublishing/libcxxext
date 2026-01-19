import urllib.request
import time
from typing import List

import lib.config as config
from lib.exec import (
    exec_or_fail,
    exec_pkgmgr_cache_update,
    find_command,
    has_stamp,
    stamp_id,
)
from lib.log import lassert_unsupported_bconf, lcheck_failed


def install(*, target_arch: str, host_platform: str, cl_name: str) -> None:
    if has_stamp("tool_cl"):
        return

    if host_platform == "linux":
        exec_pkgmgr_cache_update(host_platform)
        apt_cmd = ["sudo", "apt-get", "-o", "DPkg::Lock::Timeout=60"]

        if cl_name == "clang":
            llvm_sh_url = "https://apt.llvm.org/llvm.sh"
            llvm_sh_relp = f"./{config.tools_reldir}/llvm.sh"
            urllib.request.urlretrieve(llvm_sh_url, llvm_sh_relp)
            exec_or_fail(["chmod", "+x", llvm_sh_relp])

            # Locks apt repository, try a few times in case another process is using it.
            for i in range(10):

                def on_fail() -> None:
                    if i == 9:
                        lcheck_failed()

                exec_or_fail(["sudo", llvm_sh_relp, "19"], on_fail=on_fail)
                time.sleep(1)

            exec_or_fail(apt_cmd + ["install", "-y", "clang-19"])

            exec_or_fail(["clang-19", "--version"])
            exec_or_fail(["clang++-19", "--version"])

            if target_arch == "i686":
                exec_or_fail(
                    apt_cmd
                    + [
                        "install",
                        "-y",
                        "libc6-dev-i386",
                        "libstdc++-14-dev",
                        "gcc-14-multilib",
                        "g++-14-multilib",
                    ]
                )

                exec_or_fail(["gcc-14", "--version"])
                exec_or_fail(["g++-14", "--version"])

        elif cl_name == "gcc":
            exec_or_fail(
                apt_cmd + ["install", "-y", "gcc-14-multilib", "g++-14-multilib"]
            )

            exec_or_fail(["gcc-14", "--version"])
            exec_or_fail(["g++-14", "--version"])

        else:
            lassert_unsupported_bconf()

    elif "msys" in host_platform:
        exec_pkgmgr_cache_update(host_platform)

        pacman_cmd = ["pacman", "-S", "--needed", "--noconfirm"]
        if cl_name == "clang":
            exec_or_fail(pacman_cmd + ["mingw-w64-clang-x86_64-clang"])
            exec_or_fail(["clang", "--version"])
            exec_or_fail(["clang++", "--version"])

        elif cl_name == "gcc":
            exec_or_fail(pacman_cmd + ["mingw-w64-x86_64-gcc"])
            exec_or_fail(["x86_64-w64-mingw32-gcc", "-v"])
            exec_or_fail(["x86_64-w64-mingw32-g++", "-v"])

        else:
            lassert_unsupported_bconf()

    elif host_platform != "win32":
        lassert_unsupported_bconf()

    stamp_id("tool_cl")


def cmd_cc(*, host_platform: str, cl_name: str) -> List[str]:
    if host_platform == "linux":
        if cl_name == "clang":
            return ["clang-19"]
        elif cl_name == "gcc":
            return ["gcc-14"]
    elif "msys" in host_platform:
        if cl_name == "clang":
            return ["clang"]
        elif cl_name == "gcc":
            return ["x86_64-w64-mingw32-gcc"]
    else:
        lassert_unsupported_bconf()

    raise AssertionError


def cmd_cxx(*, host_platform: str, cl_name: str) -> List[str]:
    if host_platform == "linux":
        if cl_name == "clang":
            return ["clang++-19"]
        elif cl_name == "gcc":
            return ["g++-14"]
    elif "msys" in host_platform:
        if cl_name == "clang":
            return ["clang++"]
        elif cl_name == "gcc":
            return ["x86_64-w64-mingw32-g++"]
    else:
        lassert_unsupported_bconf()

    raise AssertionError


def cmd_gcov(*, cl_name: str) -> List[str]:
    if cl_name == "clang":
        return [
            find_command(
                [f"llvm-cov-{ver}" for ver in range(25, 18, -1)] + ["llvm-cov"]
            )
            or "[missing]",
            "gcov",
        ]
    elif cl_name == "gcc":
        return [
            find_command([f"gcov-{ver}" for ver in range(18, 12, -1)] + ["gcov"])
            or "[missing]"
        ]
    else:
        lassert_unsupported_bconf()

    raise AssertionError
