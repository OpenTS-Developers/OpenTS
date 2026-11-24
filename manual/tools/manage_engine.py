"""One contributor entry point for updating, checking, and serving the manual."""

import concurrent.futures
import difflib
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import time

try:
    import yaml
    import semantic_version
except ModuleNotFoundError:
    print("Missing Python dependencies.")
    print("Run: python -m pip install -r manual/tools/requirements.txt")
    raise SystemExit(2)

from io_utils import load_yaml
import enum_drift
import formats as format_catalogs
import validate_manual


ROOT = Path(__file__).resolve().parents[2]
MANUAL = ROOT / "manual"
TOOLS = MANUAL / "tools"
DATA = MANUAL / "data"
SITE = MANUAL / "site"
GENERATOR_SPECS = (
    ("extract.py", ("--consolidated",), "ini-keys.yaml"),
    ("scripting.py", ("--out",), "scripting.yaml"),
    ("commands.py", ("--out",), "commands.yaml"),
)


class ManualCommandError(RuntimeError):
    pass


def run_process(command, cwd=ROOT, env=None, quiet=False):
    if not quiet:
        print("RUN      " + " ".join(str(part) for part in command), flush=True)
    try:
        return subprocess.run(
            [str(part) for part in command],
            cwd=cwd,
            env=env,
            check=True,
            stdout=subprocess.DEVNULL if quiet else None,
        )
    except FileNotFoundError as error:
        raise ManualCommandError(f"command not found: {command[0]}") from error
    except subprocess.CalledProcessError as error:
        raise ManualCommandError(
            f"command failed with exit code {error.returncode}: "
            + " ".join(str(part) for part in command)) from error


def run_capture(label, command, cwd=ROOT, env=None):
    """Run a check to completion, keeping its output for ordered reporting."""

    started = time.perf_counter()
    try:
        # Astro prints box drawing and symbols the console codepage cannot
        # represent, so decode explicitly rather than through the locale.
        result = subprocess.run(
            [str(part) for part in command],
            cwd=cwd,
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            encoding="utf-8",
            errors="replace",
        )
    except FileNotFoundError as error:
        raise ManualCommandError(f"command not found: {command[0]}") from error
    return label, result.returncode, result.stdout, time.perf_counter() - started


def run_parallel(jobs):
    """Run checks that share no inputs together, reporting every failure."""

    workers = min(len(jobs), max(2, (os.cpu_count() or 4) - 1))
    with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
        submitted = [pool.submit(run_capture, *job) for job in jobs]
        results = [future.result() for future in submitted]

    # A redirected stream may use a code page without Astro's symbols.
    encoding = sys.stdout.encoding or "utf-8"
    failures = []
    for label, code, output, seconds in results:
        print(f"---- {label} ({seconds:.1f}s) ".ljust(64, "-"), flush=True)
        text = (output or "").rstrip()
        print(text.encode(encoding, errors="replace").decode(encoding), flush=True)
        if code:
            failures.append(f"{label} failed with exit code {code}")
    if failures:
        raise ManualCommandError("; ".join(failures))


def timed(label, action):
    started = time.perf_counter()
    result = action()
    print(f"TIMING   {label}: {time.perf_counter() - started:.1f}s", flush=True)
    return result


def generator_runner(script, arguments, output):
    run_process(
        [sys.executable, script, *arguments, output],
        quiet=True,
    )


def generate_files(directory, runner=generator_runner):
    directory = Path(directory)
    with concurrent.futures.ThreadPoolExecutor(len(GENERATOR_SPECS)) as pool:
        runs = [
            pool.submit(runner, TOOLS / script_name, arguments, directory / data_name)
            for script_name, arguments, data_name in GENERATOR_SPECS
        ]
    generated = {}
    for run, (script_name, _, data_name) in zip(runs, GENERATOR_SPECS):
        run.result()
        output = directory / data_name
        if not output.is_file():
            raise ManualCommandError(f"{script_name} did not create {output}")
        generated[data_name] = output
    return generated


def replace_generated(generated, data_directory=DATA):
    """Replace all generated targets, rolling back if an OS-level replace fails."""
    data_directory = Path(data_directory)
    targets = {
        name: data_directory / name for name in generated
    }
    backups = {
        name: target.read_bytes() if target.exists() else None
        for name, target in targets.items()
    }
    replaced = []
    try:
        for name, target in targets.items():
            os.replace(generated[name], target)
            replaced.append(name)
    except OSError:
        for name in reversed(replaced):
            target = targets[name]
            backup = backups[name]
            if backup is None:
                target.unlink(missing_ok=True)
                continue
            rollback = target.with_name(f".{target.name}.rollback")
            rollback.write_bytes(backup)
            os.replace(rollback, target)
        raise


def update_generated(runner=generator_runner, data_directory=DATA):
    data_directory = Path(data_directory)
    data_directory.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(
            prefix=".manual-generate-", dir=data_directory) as temporary:
        generated = generate_files(temporary, runner)
        replace_generated(generated, data_directory)


def drift_errors(generated, data_directory=DATA):
    errors = []
    for name, fresh_path in generated.items():
        committed_path = Path(data_directory) / name
        if not committed_path.exists():
            errors.append(f"manual/data/{name} is missing; run manage.py update")
            continue
        committed = committed_path.read_text(encoding="utf-8").splitlines()
        fresh = fresh_path.read_text(encoding="utf-8").splitlines()
        if committed == fresh:
            print(f"OK       manual/data/{name}")
            continue
        errors.append(f"manual/data/{name} has extraction drift; run manage.py update")
        print(f"DRIFTED  manual/data/{name}")
        diff = difflib.unified_diff(
            committed, fresh,
            fromfile=f"committed/{name}",
            tofile=f"regenerated/{name}",
            lineterm="",
        )
        for index, line in enumerate(diff):
            if index >= 40:
                print("  ... (diff truncated)")
                break
            print("  " + line)
    return errors


def git_output(*arguments):
    return subprocess.run(
        ["git", *arguments],
        cwd=ROOT,
        text=True,
        encoding="utf-8",
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )


def text_at_ref(reference, path):
    result = git_output("show", f"{reference}:{path}")
    return result.stdout if result.returncode == 0 else None


def yaml_at_ref(reference, path, required=False):
    text = text_at_ref(reference, path)
    if text is None:
        if required:
            raise ManualCommandError(f"{reference} does not contain {path}")
        return None
    return load_yaml(text)


def texts_at_ref(reference, paths):
    """Return each path's text at *reference*, or None where the path is absent.

    Line endings are normalized to LF, as text_at_ref returns them.
    """

    paths = list(paths)
    if not paths:
        return {}
    result = subprocess.run(
        ["git", "cat-file", "--batch"],
        cwd=ROOT,
        input="".join(f"{reference}:{path}\n" for path in paths).encode("utf-8"),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if result.returncode != 0:
        raise ManualCommandError(
            "git cat-file failed: "
            + result.stderr.decode("utf-8", errors="replace").strip())
    output = result.stdout
    texts = {}
    offset = 0
    for path in paths:
        end = output.index(b"\n", offset)
        header = output[offset:end].split()
        offset = end + 1
        texts[path] = None
        if len(header) != 3 or not header[2].isdigit():
            continue
        size = int(header[2])
        if header[1] == b"blob":
            texts[path] = (output[offset:offset + size].decode("utf-8")
                           .replace("\r\n", "\n").replace("\r", "\n"))
        offset += size + 1
    return texts


def _markdown_at_ref(reference, directory):
    """Return {path: text} for the Markdown files directly in *directory* at *reference*."""
    listing = git_output(
        "ls-tree", "-r", "--name-only", reference, directory)
    if listing.returncode != 0:
        raise ManualCommandError(f"cannot list {directory} at {reference}")
    paths = [
        path for path in listing.stdout.splitlines()
        if Path(path).suffix == ".md" and Path(path).parent.as_posix() == directory
    ]
    return texts_at_ref(reference, paths)


def change_records_at_ref(reference, directory="manual/changes"):
    """Return {record ID: {"data": frontmatter, "text": file text}} at *reference*.

    The default *directory* holds change records; manual/changes/rc holds RC notes.
    """
    records = {}
    for path, source in _markdown_at_ref(reference, directory).items():
        match = re.match(r"^---\r?\n([\s\S]*?)\r?\n---", source or "")
        if match:
            records[Path(path).stem] = {
                "data": load_yaml(match.group(1)) or {}, "text": source}
    return records


def changes_at_ref(reference):
    return {
        change_id: record["data"]
        for change_id, record in change_records_at_ref(reference).items()
    }


def enums_at_ref(reference):
    sources = _markdown_at_ref(reference, "manual/content/enums")
    records = {}
    for source in (sources or {}).values():
        match = re.match(r"^---\r?\n([\s\S]*?)\r?\n---", source or "")
        if match:
            data = load_yaml(match.group(1)) or {}
            if isinstance(data.get("enum_id"), str):
                records[data["enum_id"]] = data
    return records


def formats_at_ref(reference):
    sources = _markdown_at_ref(reference, "manual/content/formats")
    documents = [
        (path, source) for path, source in sources.items() if source is not None]
    if not documents:
        return None
    return format_catalogs.parse_catalog(documents)


def _commit(reference):
    result = git_output("rev-parse", "--verify", "--quiet", f"{reference}^{{commit}}")
    return result.stdout.strip() if result.returncode == 0 else None


def _is_ancestor(ancestor, descendant):
    return git_output("merge-base", "--is-ancestor", ancestor, descendant).returncode == 0


def _merge_base(first, second):
    result = git_output("merge-base", first, second)
    return result.stdout.strip() if result.returncode == 0 else None


def release_branch_name(version):
    parsed = semantic_version.Version(version)
    return f"release/{parsed.major}.{parsed.minor}"


def _branch_copies(name):
    listing = git_output("for-each-ref", "--format=%(refname)", "refs/heads", "refs/remotes")
    pattern = re.compile(rf"refs/(?:heads|remotes/[^/]+)/{re.escape(name)}")
    return [ref for ref in listing.stdout.split() if pattern.fullmatch(ref)]


def _latest_fork(tips, branches):
    """Return the newest commit of any copy of a branch that one of *tips* contains."""
    forks = {fork for tip in tips for ref in branches if (fork := _merge_base(tip, ref))}
    if not forks:
        return None
    return max(forks, key=lambda fork: sum(_is_ancestor(other, fork) for other in forks))


def release_refs(base_reference, base_releases):
    """Read the tags and release branches the release checks compare against.

    Returns released versions' tag records, the candidate's records at its release branch,
    the candidates whose branch the range from *base_reference* to HEAD merges, a
    development version that is already tagged, and an error for each missing ref. Every
    local and remote copy of a release branch counts, and an unfinished merge counts as
    merged.
    """
    def statuses(registry):
        rows = registry.get("releases") if isinstance(registry, dict) else None
        return {
            row.get("version"): row.get("status")
            for row in rows or () if isinstance(row, dict)}

    status = statuses(validate_manual.read_yaml(DATA / "releases.yaml"))
    base_status = statuses(base_releases)
    refs = {"errors": [], "tags": {}, "branches": {}, "merged": set(), "closed": None}
    head = _commit("HEAD")
    if not head:
        return refs
    tips = [head, *filter(None, [_commit("MERGE_HEAD")])]

    for version, state in status.items():
        tag = f"v{version}"
        tag_commit = _commit(f"refs/tags/{tag}")
        if state == "released":
            if not tag_commit:
                refs["errors"].append(
                    f"released version {version}: tag {tag} is missing; "
                    "fetch the tags from the OpenTS repository")
                continue
            refs["tags"][version] = {
                "tag": tag,
                "merged": any(_is_ancestor(tag_commit, tip) for tip in tips),
                "records": change_records_at_ref(tag_commit),
            }
        elif state == "development" and tag_commit and tag_commit != head and _is_ancestor(
                tag_commit, head):
            refs["closed"] = version

    candidates = {
        version for snapshot in (status, base_status)
        for version, state in snapshot.items() if state == "candidate"}
    for version in sorted(candidates):
        try:
            name = release_branch_name(version)
        except (TypeError, ValueError):
            continue
        branches = _branch_copies(name)
        if not branches:
            refs["errors"].append(
                f"candidate version {version}: branch {name} is missing; "
                "fetch it from the OpenTS repository")
            continue
        fork = _latest_fork(tips, branches)
        if status.get(version) == "candidate":
            refs["branches"][version] = {
                "branch": name,
                "records": change_records_at_ref(fork) if fork else {},
            }
        if base_reference and fork and fork != _latest_fork([base_reference], branches):
            refs["merged"].add(version)
    return refs


def load_base(reference=None):
    selected = reference or "HEAD"
    commit = git_output("cat-file", "-e", f"{selected}^{{commit}}")
    if commit.returncode != 0:
        if reference:
            raise ManualCommandError(
                f"cannot resolve requested base revision {selected!r}")
        print("NOTE     HEAD is unavailable; branch-relative checks are skipped")
        return selected, None

    enum_sources = texts_at_ref(selected, enum_drift.source_paths())
    releases = yaml_at_ref(selected, "manual/data/releases.yaml")

    return selected, {
        "keys": yaml_at_ref(selected, "manual/data/ini-keys.yaml", required=True),
        "scripting": yaml_at_ref(selected, "manual/data/scripting.yaml"),
        "commands": yaml_at_ref(selected, "manual/data/commands.yaml"),
        "formats": formats_at_ref(selected),
        "releases": releases,
        "release_refs": release_refs(selected, releases),
        "aliases": yaml_at_ref(selected, "manual/data/scripting-route-aliases.yaml"),
        "changes": changes_at_ref(selected),
        "rc_notes": set(change_records_at_ref(selected, "manual/changes/rc")),
        "enums": enums_at_ref(selected),
        "enum_signatures": enum_drift.signatures_from_sources(enum_sources),
    }


def load_working_data():
    return (
        validate_manual.read_yaml(DATA / "ini-keys.yaml"),
        validate_manual.read_yaml(DATA / "scripting.yaml"),
    )


def print_names(label, values):
    values = sorted(values)
    if values:
        print(f"  {label}: {', '.join(values)}")


def report_deltas(reference, current_keys, base_keys, current_scripting, base_scripting, base=None):
    if base_keys is None:
        return
    delta = validate_manual.classify_key_deltas(current_keys, base_keys)
    print(
        f"DELTA    {reference}: {len(delta['added'])} keys added, "
        f"{len(delta['changed'])} changed, {len(delta['removed'])} removed")
    print_names("added keys", delta["added"])
    print_names("changed keys", delta["changed"])
    print_names("removed keys", delta["removed"])

    if base_scripting is None:
        print("NOTE     base revision has no scripting dataset to compare")
        base_scripting = current_scripting
    scripting = validate_manual.classify_scripting_deltas(
        current_scripting, base_scripting)
    for kind, changes in scripting.items():
        if not any(changes.values()):
            continue
        print(
            f"DELTA    {kind}: {len(changes['added'])} added, "
            f"{len(changes['changed'])} changed, "
            f"{len(changes['removed'])} removed, "
            f"{len(changes['shifted'])} index shifts")
        print_names(f"{kind} added", changes["added"])
        print_names(f"{kind} changed", changes["changed"])
        print_names(f"{kind} removed", changes["removed"])
        if changes["shifted"]:
            shifts = [
                f"{row_id} {old}->{new}"
                for row_id, (old, new) in sorted(changes["shifted"].items())
            ]
            print(f"  {kind} index shifts: {', '.join(shifts)}")

    if base:
        base_commands = base.get("commands")
        if base_commands is None:
            print("NOTE     base revision has no command dataset to compare")
        else:
            current_commands = validate_manual.read_yaml(DATA / "commands.yaml")
            command_delta = validate_manual.classify_command_deltas(
                current_commands, base_commands)
            print(
                f"DELTA    commands: {len(command_delta['added'])} added, "
                f"{len(command_delta['changed'])} changed, "
                f"{len(command_delta['removed'])} removed")
            print_names("commands added", command_delta["added"])
            print_names("commands changed", command_delta["changed"])
            print_names("commands removed", command_delta["removed"])
            print_names("command routes changed", command_delta["route_changed"])


    if base and "enum_signatures" in base:
        current_enums = {}
        for path in (MANUAL / "content" / "enums").glob("*.md"):
            data = enum_drift.frontmatter(path)
            adapter = enum_drift.ADAPTERS.get(data.get("enum_id"))
            if adapter:
                data["_signature"] = enum_drift.signature(
                    adapter, lambda name: (ROOT / name).read_text(encoding="utf-8"))
                current_enums[data["enum_id"]] = data
        enum_changes = validate_manual.classify_enum_deltas(
            current_enums, base.get("enums") or {}, base.get("enum_signatures") or {})
        print(
            f"DELTA    enums: {len(enum_changes['added'])} added, "
            f"{len(enum_changes['changed'])} changed, "
            f"{len(enum_changes['removed'])} removed")
        print_names("enums added", enum_changes["added"])
        print_names("enums changed", enum_changes["changed"])
        print_names("enums removed", enum_changes["removed"])


def npm_command():
    npm = shutil.which("npm")
    if not npm:
        raise ManualCommandError(
            "Node/npm is missing; install the versions documented in manual/README.md")
    if not (SITE / "node_modules").is_dir():
        raise ManualCommandError(
            "site dependencies are missing; run: cd manual/site && npm ci")
    return npm


def run_site_checks(build=True):
    """Run the tests and the type check, then build and check the site.

    The tests and the repository Markdown checker read sources rather than
    build output, so they run beside the build.  The type check runs after the
    build because both regenerate the ``.astro`` type cache, and it runs beside
    the checks of the built pages.  With *build* false, the type check runs
    beside the tests and nothing is built.

    The build carries the removed-entity fixtures, whose pages are the only
    tombstones the manual has; the artifact checks then cover a superset of the
    published pages.  Proving that those pages stay out of a publishable
    artifact belongs to the workflow, which rebuilds without the fixtures and
    verifies that build before it uploads.
    """

    npm = npm_command()
    # Several test modules re-extract the whole engine catalog, so run them as
    # separate processes rather than one serial discovery.
    python_tests = [
        (f"python {path.stem}", [
            sys.executable, "-m", "unittest", "discover",
            "-s", str(TOOLS / "tests"), "-p", path.name], ROOT, None)
        for path in sorted((TOOLS / "tests").glob("test_*.py"))
    ]
    if not python_tests:
        raise ManualCommandError("no Python test modules were discovered")

    source_checks = [
        *python_tests,
        ("node tests", [npm, "test"], SITE, None),
        ("markdown links", [npm, "run", "check:docs"], SITE, None),
    ]
    if not build:
        timed("tests and type check", lambda: run_parallel([
            ("astro check", [npm, "run", "check"], SITE, None),
            *source_checks,
        ]))
        return

    fixture_environment = os.environ.copy()
    fixture_environment["MANUAL_TEST_FIXTURES"] = "1"
    # The longest job goes first so a small worker pool starts it at once.
    timed("tests and site build", lambda: run_parallel([
        ("site build", [npm, "run", "build"], SITE, fixture_environment),
        *source_checks,
    ]))
    timed("type and built-page checks", lambda: run_parallel([
        ("astro check", [npm, "run", "check"], SITE, None),
        *[
            (command, [npm, "run", command], SITE, fixture_environment)
            for command in ("check:render", "check:search", "check:links")
        ],
    ]))
    print(
        "NOTE     manual/site/dist holds the fixture build; run npm run build "
        "in manual/site for a publishable artifact", flush=True)
