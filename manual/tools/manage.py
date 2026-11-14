"""Single programmer-facing entry point for the documentation system.

``doctor`` and argument parsing stay dependency-light.  The extraction,
lifecycle, and site-check implementation is loaded only for commands that need
the installed Python toolchain.
"""

import argparse
import copy
import os
import sys
import tempfile
import time

import contributor
from extraction_history import historical_corrections


_ENGINE = None


def _engine():
    global _ENGINE
    if _ENGINE is None:
        import manage_engine

        _ENGINE = manage_engine
    # Keep the facade patchable in focused tests, notably os.replace rollback.
    _ENGINE.os = globals().get("os", _ENGINE.os)
    return _ENGINE


def __getattr__(name):
    return getattr(_engine(), name)


def generate_files(directory, runner=None):
    core = _engine()
    return core.generate_files(
        directory, core.generator_runner if runner is None else runner)


def replace_generated(generated, data_directory=None):
    core = _engine()
    return core.replace_generated(
        generated, core.DATA if data_directory is None else data_directory)


def update_generated(runner=None, data_directory=None):
    core = _engine()
    return core.update_generated(
        core.generator_runner if runner is None else runner,
        core.DATA if data_directory is None else data_directory,
    )


def drift_errors(generated, data_directory=None):
    core = _engine()
    return core.drift_errors(
        generated, core.DATA if data_directory is None else data_directory)


def _attach_extraction_history(core, reference, base, current_keys):
    if not base or base.get("keys") is None:
        return set()
    corrections = historical_corrections(
        current_keys,
        base["keys"],
        core.ROOT,
        lambda path: core.text_at_ref(reference, path),
    )
    base["extraction_corrections"] = corrections
    if corrections:
        print(f"VERIFIED {len(corrections)} historical extractor coverage corrections")
    return corrections


def _base_keys_for_reporting(base, current_keys):
    if not base or base.get("keys") is None:
        return None
    result = copy.deepcopy(base["keys"])
    for key in base.get("extraction_corrections") or ():
        if key in current_keys:
            result[key] = current_keys[key]
    return result


def _validate_working(core, base):
    errors, summary = core.validate_manual.validate_all(base)
    if errors:
        print("ACTION REQUIRED")
        for error in errors:
            print(f"  - {error}")
        return errors

    scripting = summary["scripting"]
    print(f"VERIFIED {len(summary['keys']):,} current INI keys and stable routes")
    print(
        "VERIFIED "
        + ", ".join(
            f"{len(scripting.get(table, []))} {label}"
            for label, table in core.validate_manual.SCRIPTING_TABLES.items()))
    print(f"VERIFIED {len(summary['enums'])} enum domains and source adapters")
    command_records = list(summary["commands"].values())
    release_commands = sum(
        record.get("kind") == "registered"
        and "release" in record.get("availability", {}).get("builds", [])
        for record in command_records)
    debug_commands = sum(
        record.get("kind") == "registered"
        and "debug" in record.get("availability", {}).get("builds", [])
        for record in command_records)
    print(f"VERIFIED {release_commands} Release and {debug_commands} Debug commands")
    print(f"VERIFIED {len(summary['formats'])} authored format contracts and stable routes")
    print("VERIFIED authored contracts, relations, lifecycle, and source links")
    return []


def command_update(arguments, serve=False):
    core = _engine()
    try:
        reference, base = core.load_base(arguments.base_ref)
        core.update_generated()
    except (core.ManualCommandError, OSError) as error:
        print(
            "ACTION REQUIRED\n"
            f"  - generation failed before any partial output was accepted: {error}")
        return 1

    print("GENERATED manual/data/ini-keys.yaml")
    print("GENERATED manual/data/scripting.yaml")
    print("GENERATED manual/data/commands.yaml")
    current_keys, current_scripting = core.load_working_data()
    _attach_extraction_history(core, reference, base, current_keys)
    base_keys = _base_keys_for_reporting(base, current_keys)
    base_scripting = base.get("scripting") if base else None
    core.report_deltas(
        reference,
        current_keys,
        base_keys,
        current_scripting,
        base_scripting,
        base,
    )
    errors = _validate_working(core, base)
    if errors:
        print(
            "ACTION REQUIRED\n"
            "  - Generated data is safe. Complete the scaffold/prose actions "
            "above, then run: python manual/tools/manage.py check")
        return 1

    if serve:
        try:
            npm = core.npm_command()
            core.run_process([npm, "run", "dev"], cwd=core.SITE)
        except core.ManualCommandError as error:
            print(f"ACTION REQUIRED\n  - {error}")
            return 1
    return 0


def command_check(arguments):
    core = _engine()
    try:
        reference, base = core.load_base(arguments.base_ref)
    except core.ManualCommandError as error:
        print(f"ACTION REQUIRED\n  - {error}")
        return 1

    started = time.perf_counter()
    # The regeneration is only read back for the drift comparison, so it does
    # not need to share a volume with the tracked catalogs.
    with tempfile.TemporaryDirectory(prefix=".manual-check-") as temporary:
        try:
            generated = core.timed(
                "extraction", lambda: core.generate_files(temporary))
        except core.ManualCommandError as error:
            print(f"ACTION REQUIRED\n  - extraction check failed: {error}")
            return 1
        errors = core.drift_errors(generated)

    def report():
        current_keys, current_scripting = core.load_working_data()
        _attach_extraction_history(core, reference, base, current_keys)
        core.report_deltas(
            reference,
            current_keys,
            _base_keys_for_reporting(base, current_keys),
            current_scripting,
            base.get("scripting") if base else None,
            base,
        )

    core.timed("delta report", report)
    errors.extend(core.timed("validation", lambda: _validate_working(core, base)))
    if errors:
        print("ACTION REQUIRED\n  - Run: python manual/tools/manage.py update")
        return 1

    try:
        core.run_site_checks(build=not arguments.skip_build)
    except core.ManualCommandError as error:
        print(f"ACTION REQUIRED\n  - {error}")
        return 1
    print(f"TIMING   gate total: {time.perf_counter() - started:.1f}s")
    if arguments.skip_build:
        print("VERIFIED manual check without the site build and built-page checks")
    else:
        print("VERIFIED complete manual check")
    return 0


def parser():
    result = argparse.ArgumentParser(
        description="Update, validate, and preview the OpenTS manual")
    commands = result.add_subparsers(dest="command", required=True)
    for name, help_text in (
        ("update", "regenerate data and run fast structural validation"),
        ("check", "run the complete local CI-equivalent suite"),
        ("serve", "run the fast update and start the live manual"),
    ):
        command = commands.add_parser(name, help=help_text)
        command.add_argument(
            "--base-ref",
            help="Git revision to compare against (defaults to HEAD)")
        if name == "check":
            command.add_argument(
                "--skip-build",
                action="store_true",
                help="skip the site build and the render, search and link "
                     "checks that read it")
    contributor.add_parsers(commands)
    return result


def main():
    command_parser = parser()
    arguments = command_parser.parse_args()
    contributor.validate_scaffold_arguments(command_parser, arguments)
    if arguments.command == "doctor":
        return contributor.command_doctor(arguments)
    if arguments.command == "scaffold":
        return contributor.command_scaffold(arguments)
    if arguments.command == "release-notes":
        return contributor.command_release_notes(arguments)
    if arguments.command == "update":
        return command_update(arguments)
    if arguments.command == "serve":
        return command_update(arguments, serve=True)
    return command_check(arguments)


if __name__ == "__main__":
    sys.exit(main())
