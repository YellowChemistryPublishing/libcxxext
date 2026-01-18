"""This is the primary workflow script for job:all_push."""

desc = (
    "On-push job runner workflow."
    "@warning This check will `chdir` to the supplied workdir."
)

import argparse
import json
import os
import shutil
import sys
import threading
import time
from typing import Any, List
from pathlib import Path

import lib.config as config
from lib.exec import exec_or_fail, mark_finding_ok, str_key_replace
from lib.log import lcheck_failed, lcheck_passed, lprint


def main(argv: List[str]) -> None:
    config.check_name = "job:all_push"

    parser: argparse.ArgumentParser = argparse.ArgumentParser(
        prog=config.check_name,
        usage=f"python3 {os.path.basename(__file__)} [args]...",
        description=desc,
        epilog="We don't know what we're doing™.",
    )
    parser.add_argument(
        "-d", "--workdir", help="Working directory.", metavar="", required=True
    )
    parser.add_argument(
        "-cs",
        "--checkset",
        help="Which named checkset to run.",
        metavar="",
        required=True,
    )
    parser.add_argument(
        "-c",
        "--config-path",
        help="Path to check.json configuration file.",
        metavar="",
        default="./checks.json",
    )
    parser.add_argument(
        "-v",
        "--verbose",
        help="Extra logging messages to stdout.",
        action="store_true",
    )
    parser.add_argument(
        "-s",
        "--suppress",
        help="Silence tool invocation output except on failure.",
        action="store_true",
    )
    args: argparse.Namespace = parser.parse_args()

    config.is_verbose = args.verbose

    os.chdir(args.workdir)

    os.makedirs(os.path.dirname(config.lprint_templog_relp), exist_ok=True)
    with open(config.lprint_templog_relp, "w", encoding="utf-8"):
        pass

    has_failing_check: bool = False

    with open(f"{args.workdir}/{args.config_path}", "r", encoding="utf-8") as f:
        checks_config: dict[str, Any] = json.load(f)

        checks: dict[str, dict[str, Any]] = {}
        for check in checks_config.get("checks", []):
            checks[check.get("name")] = check

        stage_index: int = 0
        for workflow in checks_config.get("on_push", []):
            key_replace_dict: dict[str, str] = workflow.get("with", {})

            if (
                str_key_replace(workflow.get("checkset"), key_replace_dict)
                != args.checkset
            ):
                continue

            for stage in workflow.get("stages", []):
                lprint(f"\x1b[47;30mRunning stage {stage_index}...\x1b[0m")

                def check_runner(
                    check: dict[str, Any], exec_results: List[Any], index: int
                ) -> Any:
                    try:
                        check_script: str = str_key_replace(
                            check.get("script", "[Missing script, this will fail!]"),
                            key_replace_dict,
                        )
                        check_with: List[str] = list(
                            map(
                                lambda s: str_key_replace(s, key_replace_dict),
                                check.get("with", []),
                            )
                        )

                        lprint(f"Running check:{Path(check_script).stem}.")

                        def on_fail() -> None:
                            raise Exception(
                                f"Note: \x1b[31;3mFailed\x1b[0m \x1b[1;3mcheck:{Path(check_script).stem}.\x1b[0m"
                            )

                        exec_or_fail(
                            [
                                "python3",
                                f"{args.workdir}/{check_script}",
                            ]
                            + (["-v"] if args.verbose else [])
                            + (["-s"] if args.suppress else [])
                            + check_with,
                            on_fail,
                        )
                    except Exception as e:
                        exec_results[index] = e

                exec_results: List[Any] = [None] * len(stage)

                thread_pool: List[threading.Thread] = []
                for i, check_name in enumerate(stage):
                    check_name = str_key_replace(check_name, key_replace_dict)
                    thread_pool.append(
                        threading.Thread(
                            target=check_runner,
                            args=(checks[check_name], exec_results, i),
                        )
                    )
                    thread_pool[-1].start()

                while len(thread_pool) > 0:
                    for thread in thread_pool:
                        if not thread.is_alive():
                            thread_pool.remove(thread)
                    time.sleep(0.1)

                is_failed: bool = False
                for result in exec_results:
                    if result is not None:
                        is_failed = True
                        lprint(str(result))
                if is_failed:
                    lprint(
                        f"\x1b[33;1mSome checks in stage {stage_index} failed.\x1b[0m"
                    )
                    has_failing_check = True

                stage_index += 1

        if stage_index == 0:
            lprint(
                "No `on_push` workflows. It would be unproductive for this to be intentional."
            )
            lcheck_failed()

    os.makedirs(config.findings_reldir, exist_ok=True)
    shutil.move(
        config.lprint_templog_relp,
        f"{config.findings_reldir}/ChecksetWorkflowOutput.log",
    )

    if not has_failing_check:
        mark_finding_ok(f"{config.findings_reldir}/ChecksetWorkflowOutput.log")
    else:
        lcheck_failed()

    lcheck_passed()


if __name__ == "__main__":
    main(sys.argv)
