"""Git pre-push hook that runs the manual check without the site build.

The check runs only when the pushed commits touch a watched path.  It reads
the working tree, not the pushed commits.
"""

from pathlib import Path
import re
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[2]
MANAGE = ROOT / "manual" / "tools" / "manage.py"
NULL_REVISION = re.compile(r"0+")
WATCHED = (
    "manual/**",
    "README.md",
    "CONTRIBUTING.md",
    "AGENTS.md",
    "LICENSE.md",
    "docs/**",
    "code/**",
    "CMakeLists.txt",
)


def git(*arguments):
    return subprocess.run(
        ["git", *arguments],
        cwd=ROOT,
        text=True,
        encoding="utf-8",
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )


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

    An update compares against the remote's previous commit.  A new branch
    lists every commit the remote lacks and compares
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
    patterns = WATCHED
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
        print("manual check skipped: the push changes no watched path")
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
    return subprocess.run(command, cwd=ROOT).returncode


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:], sys.stdin.read().splitlines()))
