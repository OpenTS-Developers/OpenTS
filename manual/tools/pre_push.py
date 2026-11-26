"""Git pre-push hook that runs the manual check without the site build.

The check runs only when the pushed commits touch a path that starts the Manual
workflow on a push.  It reads the working tree, not the pushed commits.
"""

from pathlib import Path
import os
import re
import subprocess
import sys

from io_utils import load_yaml


ROOT = Path(__file__).resolve().parents[2]
WORKFLOW = ROOT / ".github" / "workflows" / "manual-pages.yml"
MANAGE = ROOT / "manual" / "tools" / "manage.py"
NULL_REVISION = re.compile(r"0+")


def git(*arguments):
    return subprocess.run(
        ["git", *arguments],
        cwd=ROOT,
        text=True,
        encoding="utf-8",
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )


def check_environment():
    """Return this process's environment without the variables that select a repository.

    Git sets ``GIT_DIR`` for a hook run from a linked worktree.  A test that
    inherits it runs git against this repository instead of its temporary one,
    and its ``git init`` marks this repository bare.
    """

    names = set(git("rev-parse", "--local-env-vars").stdout.split())
    return {name: value for name, value in os.environ.items() if name not in names}


def workflow_paths(workflow=WORKFLOW):
    data = load_yaml(Path(workflow).read_text(encoding="utf-8"))
    # YAML 1.1 reads the bare key "on" as true.
    triggers = data.get("on", data.get(True))
    return triggers["push"]["paths"]


def path_pattern(pattern):
    """Translate a GitHub path filter, where only ``**`` crosses a slash."""

    translated = {"**": ".*", "*": "[^/]*", "?": "[^/]"}
    return re.compile("".join(
        translated.get(part, re.escape(part))
        for part in re.split(r"(\*\*|\*|\?)", pattern)))


def matching_paths(paths, patterns):
    compiled = [path_pattern(pattern) for pattern in patterns]
    return sorted(
        path for path in set(paths)
        if any(pattern.fullmatch(path) for pattern in compiled))


def is_commit(revision):
    return git("cat-file", "-e", f"{revision}^{{commit}}").returncode == 0


def pushed_paths(remote, local_sha, remote_sha):
    """Return the files the push changes and the revision to compare against.

    An update compares against the remote's previous commit, as the workflow
    does.  A new branch lists every commit the remote lacks and compares
    against its fork point from the remote's default branch, or against HEAD
    when that branch is unknown.
    """

    if not NULL_REVISION.fullmatch(remote_sha) and is_commit(remote_sha):
        listing = git("diff", "--name-only", remote_sha, local_sha)
        return listing.stdout.splitlines(), remote_sha

    listing = git(
        "log", "--format=", "--name-only", local_sha, "--not", f"--remotes={remote}")
    base = None
    default_branch = f"refs/remotes/{remote}/HEAD"
    if is_commit(default_branch):
        fork_point = git("merge-base", local_sha, default_branch)
        if fork_point.returncode == 0:
            base = fork_point.stdout.strip()
    return [path for path in listing.stdout.splitlines() if path], base


def main(arguments, updates):
    remote = arguments[0] if arguments else "origin"
    patterns = workflow_paths()
    head = git("rev-parse", "HEAD").stdout.strip()

    relevant = []
    for line in updates:
        fields = line.split()
        if len(fields) != 4 or NULL_REVISION.fullmatch(fields[1]):
            continue
        _, local_sha, _, remote_sha = fields
        paths, base = pushed_paths(remote, local_sha, remote_sha)
        if matching_paths(paths, patterns):
            relevant.append((local_sha, base))

    if not relevant:
        print("manual check skipped: the push changes no path the Manual workflow watches")
        return 0

    local_sha, base = next(
        (update for update in relevant if update[0] == head), relevant[0])
    if local_sha != head:
        print(f"NOTE     the check reads the working tree at {head[:12]}, "
              f"not the pushed commit {local_sha[:12]}")
    status = git("status", "--porcelain", "--untracked-files=no")
    dirty = matching_paths(
        (line[3:].split(" -> ")[-1] for line in status.stdout.splitlines()), patterns)
    if dirty:
        print(f"NOTE     uncommitted changes in {len(dirty)} watched files are checked too")

    print("Running the manual check without the site build; "
          "git push --no-verify skips it.", flush=True)
    command = [sys.executable, str(MANAGE), "check", "--skip-build"]
    if base:
        command += ["--base-ref", base]
    return subprocess.run(command, cwd=ROOT, env=check_environment()).returncode


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:], sys.stdin.read().splitlines()))
