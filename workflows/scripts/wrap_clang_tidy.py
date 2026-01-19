import os
import sys
from typing import List

import lib.config as config
from lib.exec import (
    exec_or_fail,
    exec_pkgmgr_cache_update,
    find_command,
    has_stamp,
    stamp_id,
)
from lib.log import lassert_unsupported_bconf


def main(argv: List[str]) -> None:
    pass


if __name__ == "__main__":
    main(sys.argv)
