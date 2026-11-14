from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

TOOLS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOLS))

import manage_engine
import versioning


def record(release, category="feature", title="Title"):
    return (
        "---\n"
        f"title: {title}\n"
        f"category: {category}\n"
        f"release: {release}\n"
        "targets: []\n"
        "credit: [Author]\n"
        "---\n"
    )


def registry(**statuses):
    rows = {
        version.replace("_", "."): {"version": version.replace("_", "."), "status": status}
        for version, status in statuses.items()
    }
    return {
        "by_version": rows,
        "development": next(
            (version for version, row in rows.items() if row["status"] == "development"), None),
        "candidate": next(
            (version for version, row in rows.items() if row["status"] == "candidate"), None),
    }


class RegistryTests(unittest.TestCase):
    def validate(self, rows, cmake="0.3.0"):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            data = root / "manual" / "data"
            data.mkdir(parents=True)
            (data / "releases.yaml").write_text(
                "releases:\n" + "".join(
                    f"  - version: {version}\n    status: {status}\n"
                    + (f"    date: '2026-01-01'\n" if status == "released" else "")
                    for version, status in rows),
                encoding="utf-8")
            (root / "CMakeLists.txt").write_text(
                f"project(OpenTS VERSION {cmake})\n"
                'set(OPENTS_VERSION_PRERELEASE "")\n',
                encoding="utf-8")
            errors = []
            result = versioning.validate_releases(errors, root / "manual", root)
            return errors, result

    def test_one_candidate_above_every_released_version(self):
        errors, result = self.validate(
            [("0.1.0", "released"), ("0.2.0", "candidate"), ("0.3.0", "development")])
        self.assertEqual(errors, [])
        self.assertEqual(result["candidate"], "0.2.0")

        errors, _ = self.validate(
            [("0.1.0", "candidate"), ("0.2.0", "released"), ("0.3.0", "development")])
        self.assertTrue(any("must be higher than every released" in error for error in errors))

        errors, _ = self.validate(
            [("0.1.0", "candidate"), ("0.2.0", "candidate"), ("0.3.0", "development")])
        self.assertTrue(any("at most one candidate" in error for error in errors))

    def test_status_transitions(self):
        def transition(before, after):
            errors = []
            versioning.validate_released_registry_history(errors, after, before)
            return errors

        cut = registry(**{"0_2_0": "candidate", "0_3_0": "development"})
        self.assertEqual(transition(registry(**{"0_2_0": "development"}), cut), [])
        sealed = registry(**{"0_2_0": "released", "0_3_0": "development"})
        self.assertEqual(transition(cut, sealed), [])
        self.assertEqual(transition(cut, registry(**{"0_3_0": "development"})), [])
        self.assertEqual(transition(
            registry(**{"0_3_0": "development"}),
            registry(**{"0_2_1": "candidate", "0_3_0": "development"})), [])
        self.assertTrue(any(
            "cannot return to development" in error
            for error in transition(cut, registry(**{"0_2_0": "development"}))))
        self.assertTrue(any(
            "cannot be demoted" in error for error in transition(sealed, cut)))


class ChangeAssignmentTests(unittest.TestCase):
    def validate(self, files, current, base, base_changes):
        with tempfile.TemporaryDirectory() as temporary:
            manual = Path(temporary)
            (manual / "changes").mkdir()
            for change_id, release in files.items():
                (manual / "changes" / f"{change_id}.md").write_text(
                    record(release), encoding="utf-8")
            errors = []
            versioning.validate_changes(
                errors, manual, current, {}, {}, {}, [], base_changes, base)
            return errors

    def test_new_records_may_target_the_candidate(self):
        state = registry(**{"0_1_0": "released", "0_2_0": "candidate", "0_3_0": "development"})
        self.assertEqual(self.validate({"fix": "0.2.0"}, state, state, {}), [])
        errors = self.validate({"fix": "0.1.0"}, state, state, {})
        self.assertTrue(any(
            "development release 0.3.0 or the candidate release 0.2.0" in error
            for error in errors))

    def test_records_move_only_between_open_versions(self):
        state = registry(**{"0_1_0": "released", "0_2_0": "candidate", "0_3_0": "development"})
        base_changes = {
            "backport": {"title": "Title", "category": "feature", "release": "0.3.0",
                         "targets": [], "credit": ["Author"]},
            "old": {"title": "Title", "category": "feature", "release": "0.1.0",
                    "targets": [], "credit": ["Author"]},
        }
        self.assertEqual(self.validate(
            {"backport": "0.2.0", "old": "0.1.0"}, state, state, base_changes), [])
        errors = self.validate({"backport": "0.3.0", "old": "0.3.0"}, state, state, base_changes)
        self.assertEqual(
            [error for error in errors if "release assignment is stable" in error],
            ["changes/old.md: release assignment is stable and cannot be changed "
             "from '0.1.0' to '0.3.0'"])

    def test_a_push_that_seals_a_version_may_still_bring_its_records(self):
        base = registry(**{"0_2_0": "candidate", "0_3_0": "development"})
        current = registry(**{"0_2_0": "released", "0_3_0": "development"})
        self.assertEqual(self.validate({"late-fix": "0.2.0"}, current, base, {}), [])


class ReleaseRefTests(unittest.TestCase):
    def changes(self, manual, **records):
        (manual / "changes").mkdir(exist_ok=True)
        result = {}
        for change_id, (release, text) in records.items():
            path = manual / "changes" / f"{change_id}.md"
            path.write_text(text, encoding="utf-8")
            result[change_id] = {"path": path, "data": {
                "release": release, "category": "feature", "targets": [], "breaking": False}}
        return result

    def check(self, changes, refs):
        errors = []
        versioning.validate_release_refs(errors, changes, refs)
        return errors

    def test_candidate_records_mirror_the_release_branch(self):
        with tempfile.TemporaryDirectory() as temporary:
            manual = Path(temporary)
            changes = self.changes(
                manual, same=("0.2.0", record("0.2.0")), edited=("0.2.0", record("0.2.0")),
                stray=("0.2.0", record("0.2.0")), feature=("0.3.0", record("0.3.0")))
            branch = {
                "same": {"data": {"release": "0.2.0"}, "text": record("0.2.0").replace(
                    "\n", "\r\n")},
                "edited": {"data": {"release": "0.2.0"}, "text": record("0.2.0", title="Old")},
                "only-there": {"data": {"release": "0.2.0"}, "text": record("0.2.0")},
            }
            errors = self.check(changes, {
                "branches": {"0.2.0": {"branch": "release/0.2", "records": branch}}})
            self.assertEqual(sorted(error.split(":")[0] for error in errors),
                             ["edited", "only-there", "stray"])

    def test_released_records_keep_their_tagged_lifecycle(self):
        with tempfile.TemporaryDirectory() as temporary:
            manual = Path(temporary)
            changes = self.changes(
                manual, reworded=("0.1.0", record("0.1.0", title="New")),
                recategorized=("0.1.0", record("0.1.0")), added=("0.1.0", record("0.1.0")))
            tagged = {
                "reworded": {"data": {"release": "0.1.0", "category": "feature",
                                      "targets": []}},
                "recategorized": {"data": {"release": "0.1.0", "category": "fix",
                                           "targets": []}},
                "dropped": {"data": {"release": "0.1.0", "category": "fix", "targets": []}},
            }
            errors = self.check(changes, {"tags": {"0.1.0": {
                "tag": "v0.1.0", "merged": True, "records": tagged}}})
            self.assertEqual(sorted(errors), [
                "added: release 0.1.0 has no such record at v0.1.0",
                "dropped: record of released version 0.1.0 is missing",
                "recategorized: category differs from the released record at v0.1.0",
            ])
            errors = self.check(changes, {"tags": {"0.1.0": {
                "tag": "v0.1.0", "merged": False, "records": tagged}}})
            self.assertEqual(errors, [
                "released version 0.1.0: tag v0.1.0 is not merged into HEAD"])

    def test_a_tagged_development_version_closes_the_branch(self):
        errors = self.check({}, {"closed": "0.2.0", "errors": ["missing ref"]})
        self.assertEqual(errors, [
            "missing ref",
            "version 0.2.0 is already tagged v0.2.0; open the next version before "
            "committing here",
        ])


class ReleaseRefGitTests(unittest.TestCase):
    def git(self, *arguments):
        subprocess.run(
            ["git", "-c", "user.name=Test", "-c", "user.email=test@example.invalid",
             "-c", "commit.gpgsign=false", "-c", "tag.gpgsign=false", *arguments],
            cwd=self.root, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    def write(self, path, text):
        target = self.root / path
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(text, encoding="utf-8", newline="\n")

    def commit(self, message):
        self.git("add", "-A")
        self.git("commit", "-q", "-m", message)

    def releases(self, **statuses):
        self.write("manual/data/releases.yaml", "releases:\n" + "".join(
            f"  - version: {version.replace('_', '.')}\n    status: {status}\n"
            + ("    date: '2026-01-01'\n" if status == "released" else "")
            for version, status in statuses.items()))

    def refs(self, base="HEAD"):
        with mock.patch.object(manage_engine, "ROOT", self.root), \
                mock.patch.object(manage_engine, "DATA", self.root / "manual" / "data"):
            base_releases = manage_engine.yaml_at_ref(base, "manual/data/releases.yaml")
            return manage_engine.release_refs(base, base_releases)

    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.git("init", "-q", "-b", "main")
        self.releases(**{"0_1_0": "released", "0_2_0": "development"})
        self.write("manual/changes/first.md", record("0.1.0"))
        self.commit("First")
        self.git("tag", "v0.1.0")
        self.write("manual/changes/second.md", record("0.2.0"))
        self.commit("Second")
        self.git("branch", "release/0.2")

    def tearDown(self):
        self.temporary.cleanup()

    def test_released_tags_are_read_and_a_missing_tag_is_an_error(self):
        refs = self.refs()
        self.assertEqual(refs["errors"], [])
        self.assertTrue(refs["tags"]["0.1.0"]["merged"])
        self.assertEqual(set(refs["tags"]["0.1.0"]["records"]), {"first"})
        self.git("tag", "-d", "v0.1.0")
        self.assertTrue(any("tag v0.1.0 is missing" in error for error in self.refs()["errors"]))

    def test_candidate_branch_records_and_merge_detection(self):
        self.releases(**{"0_1_0": "released", "0_2_0": "candidate", "0_3_0": "development"})
        self.commit("Open 0.3.0")
        refs = self.refs()
        self.assertEqual(set(refs["branches"]["0.2.0"]["records"]), {"first", "second"})
        self.assertEqual(refs["merged"], set())

        self.git("checkout", "-q", "release/0.2")
        self.write("manual/changes/branch-fix.md", record("0.2.0"))
        self.commit("Fix on the branch")
        self.git("checkout", "-q", "main")
        before = subprocess.run(
            ["git", "rev-parse", "HEAD"], cwd=self.root, text=True,
            stdout=subprocess.PIPE, check=True).stdout.strip()
        self.git("merge", "-q", "--no-ff", "--no-edit", "release/0.2")
        refs = self.refs(before)
        self.assertEqual(refs["merged"], {"0.2.0"})
        self.assertIn("branch-fix", refs["branches"]["0.2.0"]["records"])

    def rev(self, reference):
        return self.git_output("rev-parse", reference)

    def git_output(self, *arguments):
        return subprocess.run(
            ["git", *arguments], cwd=self.root, text=True, stdout=subprocess.PIPE,
            check=True).stdout.strip()

    def branch_fix(self, name):
        self.git("checkout", "-q", "release/0.2")
        self.write(f"manual/changes/{name}.md", record("0.2.0"))
        self.commit(name)
        self.git("checkout", "-q", "main")

    def open_next(self):
        self.releases(**{"0_1_0": "released", "0_2_0": "candidate", "0_3_0": "development"})
        self.commit("Open 0.3.0")

    def test_any_remote_copy_of_the_branch_counts(self):
        self.open_next()
        self.branch_fix("fix")
        self.git("update-ref", "refs/remotes/upstream/release/0.2", self.rev("release/0.2"))
        self.git("branch", "-D", "release/0.2")
        before = self.rev("HEAD")
        self.git("merge", "-q", "--no-ff", "--no-edit", "upstream/release/0.2")
        refs = self.refs(before)
        self.assertEqual(refs["errors"], [])
        self.assertIn("fix", refs["branches"]["0.2.0"]["records"])
        self.assertEqual(refs["merged"], {"0.2.0"})

    def test_the_newest_copy_wins_over_a_stale_origin(self):
        self.open_next()
        self.git("update-ref", "refs/remotes/origin/release/0.2", self.rev("release/0.2"))
        self.branch_fix("fix")
        self.git("merge", "-q", "--no-ff", "--no-edit", "release/0.2")
        self.assertIn("fix", self.refs()["branches"]["0.2.0"]["records"])

    def test_a_merge_in_progress_counts_as_merged(self):
        self.open_next()
        self.branch_fix("fix")
        self.git("merge", "-q", "--no-ff", "--no-commit", "release/0.2")
        refs = self.refs()
        self.assertIn("fix", refs["branches"]["0.2.0"]["records"])
        self.assertEqual(refs["merged"], {"0.2.0"})

    def test_missing_release_branch_is_an_error(self):
        self.releases(**{"0_1_0": "released", "0_2_0": "candidate", "0_3_0": "development"})
        self.commit("Open 0.3.0")
        self.git("branch", "-D", "release/0.2")
        self.assertTrue(any(
            "branch release/0.2 is missing" in error for error in self.refs()["errors"]))

    def test_tagged_development_version_closes_later_commits(self):
        self.git("checkout", "-q", "release/0.2")
        self.git("tag", "v0.2.0")
        self.assertIsNone(self.refs()["closed"])
        self.write("manual/changes/late.md", record("0.2.0"))
        self.commit("Late fix")
        self.assertEqual(self.refs()["closed"], "0.2.0")


if __name__ == "__main__":
    unittest.main()
