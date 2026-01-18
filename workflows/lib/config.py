"""Configuration for workflows."""

from typing import List

# The occasional downloads.

gcovr_url_linux: str = (
    "https://github.com/gcovr/gcovr/releases/download/8.5/gcovr-8.5-linux-x86_64"
)
gcovr_url_msys: str = (
    "https://github.com/gcovr/gcovr/releases/download/8.5/gcovr-8.5-win-amd64.exe"
)

# Static config.

tools_reldir: str = "tooling-build"
findings_reldir: str = "findings-build"
lprint_templog_relp: str = "tooling-build/lprint_tempout.log"

# Probably no touchy.

support_archs: List[str] = ["x86_64", "i686"]
support_platforms: List[str] = ["linux", "win32", "msys64", "msysclang64", "macos"]
support_compilers: List[str] = ["clang", "gcc", "msvc"]

# Set in scripts.

check_name: str = "check:unset"
is_verbose: bool = False
is_silent: bool = False
