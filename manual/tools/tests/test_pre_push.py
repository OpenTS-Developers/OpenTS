from pathlib import Path
import sys
import unittest


TOOLS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOLS))

import pre_push


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

    def test_watched_paths_include_the_manual(self):
        self.assertTrue(pre_push.matching_paths(
            ["manual/tools/manage.py"], pre_push.WATCHED))


if __name__ == "__main__":
    unittest.main()
