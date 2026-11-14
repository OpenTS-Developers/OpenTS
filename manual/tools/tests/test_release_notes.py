import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

TOOLS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOLS))

import contributor_engine
import manage_engine


def record(name, directory, title, category, release, breaking=False,
           migration=(), credit=()):
    lines = ["---", f"title: {title}", f"category: {category}",
             f"release: {release}"]
    if breaking:
        lines.append("breaking: true")
    if migration:
        lines.append("migration:")
        lines.extend(f"- {step}" for step in migration)
    if credit:
        lines.append("credit: [" + ", ".join(credit) + "]")
    lines += ["---", "", "Body prose.", ""]
    (directory / f"{name}.md").write_text("\n".join(lines), encoding="utf-8")


class ReleaseNotesTests(unittest.TestCase):
    def test_orders_sections_and_appends_credits(self):
        with tempfile.TemporaryDirectory() as temporary:
            manual = Path(temporary)
            changes = manual / "changes"
            changes.mkdir()
            record("b-fix", changes, "Fix the thing", "fix", "0.1.0",
                   credit=("ZivDero", "ts-patches contributors"))
            record("a-feature", changes, "Add the thing", "feature", "0.1.0")
            record("c-breaking", changes, "Break the thing", "feature", "0.1.0",
                   breaking=True, migration=("Rename the key.",))
            record("later", changes, "Future work", "fix", "0.2.0")
            with mock.patch.object(contributor_engine, "MANUAL", manual):
                notes = contributor_engine.release_notes("0.1.0")

        self.assertEqual(notes, "\n".join([
            "### Breaking changes",
            "",
            "- Break the thing",
            "  - Migration: Rename the key.",
            "",
            "### New features",
            "",
            "- Add the thing",
            "- Break the thing",
            "",
            "### Bug fixes",
            "",
            "- Fix the thing (by ZivDero, ts-patches contributors)",
            "",
        ]))

    def test_breaking_without_migration_keeps_the_change(self):
        with tempfile.TemporaryDirectory() as temporary:
            manual = Path(temporary)
            changes = manual / "changes"
            changes.mkdir()
            record("removed", changes, "Remove a mode", "feature", "0.1.0",
                   breaking=True)
            with mock.patch.object(contributor_engine, "MANUAL", manual):
                notes = contributor_engine.release_notes("0.1.0")

        self.assertIn("### Breaking changes\n\n- Remove a mode", notes)
        self.assertNotIn("Migration:", notes)

    def test_refuses_a_release_no_record_targets(self):
        with tempfile.TemporaryDirectory() as temporary:
            manual = Path(temporary)
            (manual / "changes").mkdir()
            with mock.patch.object(contributor_engine, "MANUAL", manual):
                with self.assertRaises(ValueError):
                    contributor_engine.release_notes("9.9.9")


class TaggedReleaseNotesTests(unittest.TestCase):
    def git(self, *arguments):
        return subprocess.run(
            ["git", "-c", "user.name=Test", "-c", "user.email=test@example.invalid",
             "-c", "commit.gpgsign=false", "-c", "tag.gpgsign=false", *arguments],
            cwd=self.root, check=True, text=True, stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL).stdout

    def change(self, name, title, release="0.2.0", category="fix", folder="changes"):
        directory = self.root / "manual" / folder
        directory.mkdir(parents=True, exist_ok=True)
        record(name, directory, title, category, release)

    def commit(self, message, tag=None):
        self.git("add", "-A")
        self.git("commit", "-q", "--allow-empty", "-m", message)
        if tag:
            self.git("tag", tag)

    def notes(self, tag, environment=None):
        with mock.patch.object(manage_engine, "ROOT", self.root), \
                mock.patch.dict(os.environ, environment or {}, clear=False):
            if environment is None:
                os.environ.pop("GITHUB_SERVER_URL", None)
                os.environ.pop("GITHUB_REPOSITORY", None)
            return contributor_engine.release_notes("0.2.0", tag)

    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.git("init", "-q", "-b", "main")
        self.change("feature", "Add the feature", category="feature")
        self.change("old", "Older work", release="0.1.0")
        self.commit("Cut", "v0.2.0-rc1")

    def tearDown(self):
        self.temporary.cleanup()

    def test_first_candidate_lists_every_change(self):
        self.assertEqual(self.notes("v0.2.0-rc1"),
                         "### New features\n\n- Add the feature\n")

    def test_later_candidate_lists_new_and_edited_records_and_new_rc_notes(self):
        self.change("feature", "Add the finished feature", category="feature")
        self.change("crash", "Fix a crash")
        self.change("tester", "Fix the menu flicker", folder="changes/rc")
        self.commit("Fixes", "v0.2.0-rc2")
        notes = self.notes("v0.2.0-rc2", {
            "GITHUB_SERVER_URL": "https://example.invalid", "GITHUB_REPOSITORY": "o/r"})
        updated, full = notes.split("<details>")
        self.assertEqual(updated, "\n".join([
            "### Updated since 0.2.0-rc1",
            "",
            "- Add the finished feature",
            "- Fix a crash",
            "- Fix the menu flicker",
            "",
            "[Commits since 0.2.0-rc1]"
            "(https://example.invalid/o/r/compare/v0.2.0-rc1...v0.2.0-rc2)",
            "",
            "",
        ]))
        self.assertIn("### Bug fixes\n\n- Fix a crash\n", full)
        self.assertNotIn("menu flicker", full)

    def test_final_lists_record_edits_since_the_last_candidate_but_no_rc_notes(self):
        self.change("tester", "Fix the menu flicker", folder="changes/rc")
        self.commit("Note", "v0.2.0-rc2")
        self.change("crash", "Fix a crash")
        self.commit("Fix")
        self.git("tag", "v0.2.0")
        self.git("tag", "v0.2.0-rc3")
        notes = self.notes("v0.2.0")
        self.assertTrue(notes.startswith("### New features"))
        self.assertIn("### Updated since 0.2.0-rc3\n\nNo change records changed.\n", notes)
        self.assertNotIn("menu flicker", notes)
        self.assertNotIn("[Commits since", notes)

    def test_a_later_label_on_the_same_commit_is_not_the_predecessor(self):
        self.git("tag", "v0.2.0-rc2")
        self.assertIn("Updated since 0.2.0-rc1", self.notes("v0.2.0-rc2"))
        self.assertNotIn("Updated since", self.notes("v0.2.0-rc1"))

    def test_rc10_follows_rc9_and_predecessors_are_reached_through_merges(self):
        self.git("checkout", "-q", "-b", "release/0.2")
        self.commit("Branch fix", "v0.2.0-rc9")
        self.git("checkout", "-q", "main")
        self.commit("Main work")
        self.git("merge", "-q", "--no-ff", "--no-edit", "release/0.2")
        self.git("tag", "v0.2.0-rc10")
        self.assertIn("Updated since 0.2.0-rc9", self.notes("v0.2.0-rc10"))

    def test_tag_must_match_the_version_and_exist(self):
        with self.assertRaises(ValueError):
            self.notes("v0.3.0-rc1")
        with self.assertRaises(ValueError):
            self.notes("v0.2.0-rc7")


if __name__ == "__main__":
    unittest.main()
