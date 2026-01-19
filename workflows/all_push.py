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
from concurrent.futures import Future, ThreadPoolExecutor, as_completed
from typing import Any

import lib.config as config
from lib.exec import exec_or_fail, mark_finding_ok, str_key_replace
from lib.log import lcheck_failed, lcheck_passed, lprint


def run_check(
    check_name: str,
    check: dict[str, Any],
    key_replace_dict: dict[str, str],
    workdir: str,
    verbose: bool,
    suppress: bool,
) -> None:
    check_script = str_key_replace(
        check.get("script", "[Missing script, this will fail!]"), key_replace_dict
    )
    check_args = [
        str_key_replace(arg, key_replace_dict) for arg in check.get("with", [])
    ]
    lprint(f"Running {check_name}.")

    def on_fail() -> None:
        raise Exception(f"Note: \x1b[31;3mFailed\x1b[0m \x1b[1;3m{check_name}.\x1b[0m")

    exec_or_fail(
        [
            sys.executable,
            f"{workdir}/{check_script}",
            *(((["-v"] if verbose else []))),
            *(((["-s"] if suppress else []))),
            *check_args,
        ],
        on_fail=on_fail,
    )


def main(argv: list[str]) -> None:
    config.check_name = "job:all_push"

    parser = argparse.ArgumentParser(
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
        "-v", "--verbose", help="Extra logging messages to stdout.", action="store_true"
    )
    parser.add_argument(
        "-s",
        "--suppress",
        help="Silence tool invocation output except on failure.",
        action="store_true",
    )
    args = parser.parse_args()

    config.is_verbose = args.verbose
    os.chdir(args.workdir)
    os.makedirs(config.findings_reldir, exist_ok=True)

    os.makedirs(os.path.dirname(config.lprint_templog_relp), exist_ok=True)
    with open(config.lprint_templog_relp, "w", encoding="utf-8"):
        pass

    with open(f"{args.workdir}/{args.config_path}", "r", encoding="utf-8") as f:
        checks_config = json.load(f)
        checks = {check["name"]: check for check in checks_config.get("checks", [])}

        for workflow in checks_config.get("on_push", []):
            key_replace_dict = workflow.get("with", {})

            if (
                str_key_replace(workflow.get("checkset"), key_replace_dict)
                != args.checkset
            ):
                continue

            chains = workflow.get("chains", {})
            checks_and_deps = {
                str_key_replace(name, key_replace_dict): [
                    str_key_replace(dep, key_replace_dict) for dep in deps
                ]
                for name, deps in chains.items()
            }

            completed: set[str] = set()
            failed: set[str] = set()

            def is_ready(check_name: str) -> bool:
                if check_name in completed | failed:
                    return False
                deps = checks_and_deps[check_name]
                return all(dep in completed for dep in deps) and not any(
                    dep in failed for dep in deps
                )

            pending_futures: dict[str, Future[None]] = {}

            with ThreadPoolExecutor() as executor:
                while len(completed) + len(failed) < len(checks_and_deps):
                    for check_name in checks_and_deps:
                        if is_ready(check_name) and check_name not in pending_futures:
                            future = executor.submit(
                                run_check,
                                check_name,
                                checks[check_name],
                                key_replace_dict,
                                args.workdir,
                                args.verbose,
                                args.suppress,
                            )
                            pending_futures[check_name] = future

                    if not pending_futures:
                        break

                    done_futures = set()
                    for future in as_completed(pending_futures.values()):
                        done_futures.add(future)
                        break

                    for future in done_futures:
                        check_name = next(
                            name for name, f in pending_futures.items() if f == future
                        )
                        del pending_futures[check_name]

                        try:
                            future.result()
                            completed.add(check_name)
                        except Exception as e:
                            failed.add(check_name)
                            lprint(str(e))

            skipped = set(checks_and_deps.keys()) - completed - failed
            if skipped:
                lprint(
                    f"\x1b[33;1mSkipped {len(skipped)} check(s) due to failed dependencies.\x1b[0m"
                )
                for check_name in sorted(skipped):
                    lprint(f"  - {check_name}")

            has_failures = len(failed) > 0
            break

        else:
            lprint(
                "No `on_push` workflows. It would be unproductive for this to be intentional."
            )
            lcheck_failed()

    log_path = f"{config.findings_reldir}/ChecksetWorkflowOutput.log"
    shutil.move(config.lprint_templog_relp, log_path)

    if has_failures:
        lcheck_failed()

    mark_finding_ok(log_path)
    lcheck_passed()


if __name__ == "__main__":
    main(sys.argv)
