import os
import sys
from concurrent.futures import ThreadPoolExecutor
from typing import List

sys.path.append(os.path.abspath(f"{os.path.dirname(__file__)}/.."))

import lib.config as config
from lib.exec import exec_or_fail
from lib.log import lcheck_failed, lcheck_passed, lprint


def main(argv: List[str]) -> None:
    config.check_name = "scripts:clang-tidy"

    srcs: List[str] = []
    other_args: List[str] = []

    keep_finding_srcs = True
    for arg in argv[2:]:
        if arg == "--":
            other_args.append("--")
            keep_finding_srcs = False
            continue
        if arg.startswith("-"):
            other_args.append(arg)
            continue

        if keep_finding_srcs:
            srcs.append(arg)
        else:
            other_args.append(arg)

    if not srcs:
        return

    clang_tidy_cmd = argv[1]
    with_threads = os.cpu_count() or 4

    batches = [srcs[i::with_threads] for i in range(with_threads)]
    batches = [batch for batch in batches if batch]

    def lint_batch(batch: List[str]) -> None:
        def on_fail() -> None:
            raise Exception(
                f"`clang-tidy` produced non-zero exit code processing {', '.join([f'`{s}`' for s in batch])}"
            )

        exec_or_fail([clang_tidy_cmd] + batch + other_args, on_fail=on_fail)

    with ThreadPoolExecutor(max_workers=with_threads) as executor:
        futures = [executor.submit(lint_batch, batch) for batch in batches]

        for future in futures:
            try:
                future.result()
            except Exception as e:
                lprint(str(e))
                lcheck_failed()

    lcheck_passed()


if __name__ == "__main__":
    main(sys.argv)
