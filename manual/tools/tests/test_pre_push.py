import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest import mock


TOOLS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOLS))

import pre_push
from git_sandbox import isolate


class RepositoryVariableTests(unittest.TestCase):
    def test_check_environment_drops_the_repository_variables(self):
        with mock.patch.dict(os.environ, {
                "GIT_DIR": "elsewhere/.git", "GIT_WORK_TREE": "elsewhere",
                "GIT_INDEX_FILE": "elsewhere/index", "MANUAL_TEST_KEEP": "1"}):
            environment = pre_push.check_environment()
        self.assertNotIn("GIT_DIR", environment)
        self.assertNotIn("GIT_WORK_TREE", environment)
        self.assertNotIn("GIT_INDEX_FILE", environment)
        self.assertEqual(environment["MANUAL_TEST_KEEP"], "1")

    def test_an_isolated_git_init_leaves_the_inherited_repository_alone(self):
        isolate(self)
        with tempfile.TemporaryDirectory() as folder:
            outside = Path(folder) / "outside"
            inside = Path(folder) / "inside"
            outside.mkdir()
            inside.mkdir()
            subprocess.run(["git", "init", "-q"], cwd=outside, check=True)
            os.environ["GIT_DIR"] = str(outside / ".git")
            isolate(self)
            subprocess.run(["git", "init", "-q"], cwd=inside, check=True)
            bare = subprocess.run(
                ["git", "config", "--file", str(outside / ".git" / "config"),
                 "core.bare"],
                text=True, stdout=subprocess.PIPE, check=True).stdout.strip()
            self.assertEqual(bare, "false")
            self.assertTrue((inside / ".git").is_dir())


class PrePushPathTests(unittest.TestCase):
    def test_only_double_star_crosses_a_directory(self):
        paths = [
            "manual/data/ini-keys.yaml",
            "README.md",
            "docs/README.md",
            "docsextra/notes.md",
            "code/rules.cpp",
            "code/sub/rules.cpp",
        ]
        self.assertEqual(
            pre_push.matching_paths(paths, ["manual/**", "README.md", "code/*.cpp"]),
            ["README.md", "code/rules.cpp", "manual/data/ini-keys.yaml"])
        self.assertEqual(pre_push.matching_paths(paths, ["docs/**"]), ["docs/README.md"])

    def test_push_filter_is_read_through_the_yaml_boolean_on_key(self):
        with tempfile.TemporaryDirectory() as folder:
            workflow = Path(folder) / "workflow.yml"
            workflow.write_text(
                "on:\n"
                "  pull_request:\n"
                "    paths: [\"other/**\"]\n"
                "  push:\n"
                "    paths:\n"
                "      - \"manual/**\"\n"
                "      - \"code/**\"\n",
                encoding="utf-8")
            self.assertEqual(pre_push.workflow_paths(workflow), ["manual/**", "code/**"])

    def test_repository_workflow_watches_the_manual(self):
        self.assertTrue(pre_push.matching_paths(
            ["manual/tools/manage.py"], pre_push.workflow_paths()))


if __name__ == "__main__":
    unittest.main()
