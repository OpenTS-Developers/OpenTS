#!/usr/bin/env python3
"""Tests for the shared OpenTS writing-style hook."""

from __future__ import annotations

import importlib.util
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


SCRIPT = Path(__file__).with_name("style-rules.py")
SPEC = importlib.util.spec_from_file_location("opents_style_rules", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
STYLE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = STYLE
SPEC.loader.exec_module(STYLE)


class HookTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        (self.root / "code").mkdir()
        (self.root / "manual").mkdir()
        (self.root / "docs").mkdir()
        (self.root / "AGENTS.md").write_text(
            "# Root\n\n## Writing prose\n\nUse plain English.\n\n## Source\n",
            encoding="utf-8",
        )
        (self.root / "code" / "AGENTS.md").write_text(
            "# Code\n\n## Comments\n\nExplain only hidden constraints.\n\n## Tests\n",
            encoding="utf-8",
        )

    def tearDown(self) -> None:
        self.temp.cleanup()

    def post(self, tool_name: str, tool_input: dict, **extra: object) -> dict | None:
        payload = {
            "hook_event_name": "PostToolUse",
            "tool_name": tool_name,
            "tool_input": tool_input,
            "cwd": str(self.root),
            **extra,
        }
        return STYLE.handle_payload(payload, self.root)

    def test_required_sections_validate(self) -> None:
        self.assertEqual(STYLE.validate_rule_sections(self.root), [])
        (self.root / "AGENTS.md").write_text("# Root\n", encoding="utf-8")
        self.assertEqual(
            STYLE.validate_rule_sections(self.root),
            ["AGENTS.md is missing `## Writing prose`"],
        )

    def test_section_extraction_stops_at_next_h2(self) -> None:
        section = STYLE.extract_rule_section(self.root, Path("AGENTS.md"), "Writing prose")
        self.assertEqual(section, "## Writing prose\n\nUse plain English.")

    def test_markdown_edit_gets_prose_reminder_only(self) -> None:
        result = self.post(
            "Edit",
            {
                "file_path": str(self.root / "docs" / "guide.md"),
                "old_string": "Old.\n",
                "new_string": "New.\n",
            },
        )
        assert result is not None
        self.assertNotIn("decision", result)
        context = result["hookSpecificOutput"]["additionalContext"]
        self.assertIn("## Writing prose", context)
        self.assertNotIn("## Comments", context)

    def test_manual_markdown_reminder_points_to_manual_rules(self) -> None:
        result = self.post(
            "Edit",
            {
                "file_path": str(self.root / "manual" / "README.md"),
                "old_string": "Old.\n",
                "new_string": "New.\n",
            },
        )
        assert result is not None
        self.assertIn("manual/AGENTS.md", result["hookSpecificOutput"]["additionalContext"])

    def test_source_edit_without_comments_is_ignored(self) -> None:
        result = self.post(
            "Edit",
            {
                "file_path": str(self.root / "code" / "example.cpp"),
                "old_string": "int value = 1;\n",
                "new_string": "int value = 2;\n",
            },
        )
        self.assertIsNone(result)

    def test_clean_comment_gets_reminder_without_block(self) -> None:
        result = self.post(
            "Edit",
            {
                "file_path": str(self.root / "code" / "example.cpp"),
                "old_string": "entry.Refresh();\n",
                "new_string": (
                    "// Preserve order for wire compatibility.\n"
                    "entry.Refresh();\n"
                ),
            },
        )
        assert result is not None
        self.assertNotIn("decision", result)
        self.assertIn("## Comments", result["hookSpecificOutput"]["additionalContext"])

    def test_prose_triple_slash_blocks(self) -> None:
        result = self.post(
            "Edit",
            {
                "file_path": str(self.root / "code" / "example.cpp"),
                "old_string": "void Run();\n",
                "new_string": "/// Runs the operation.\nvoid Run();\n",
            },
        )
        assert result is not None
        self.assertEqual(result["decision"], "block")
        self.assertIn("without XML tags", result["reason"])

    def test_edit_narration_blocks(self) -> None:
        result = self.post(
            "Edit",
            {
                "file_path": str(self.root / "code" / "example.cpp"),
                "old_string": "void Run();\n",
                "new_string": "// This change removes the old path.\nvoid Run();\n",
            },
        )
        assert result is not None
        self.assertEqual(result["decision"], "block")
        self.assertIn("narrates the edit", result["reason"])

    def test_comment_that_restates_code_blocks(self) -> None:
        result = self.post(
            "Edit",
            {
                "file_path": str(self.root / "code" / "example.cpp"),
                "old_string": "entry.Refresh();\n",
                "new_string": "// Refresh entry.\nentry.Refresh();\n",
            },
        )
        assert result is not None
        self.assertEqual(result["decision"], "block")
        self.assertIn("restates the adjacent code", result["reason"])

    def test_write_without_old_content_reminds_but_does_not_block(self) -> None:
        result = self.post(
            "Write",
            {
                "file_path": str(self.root / "code" / "example.cpp"),
                "content": "// This change removes the old path.\nvoid Run();\n",
            },
        )
        assert result is not None
        self.assertNotIn("decision", result)
        self.assertIn("## Comments", result["hookSpecificOutput"]["additionalContext"])

    def test_write_can_use_old_content_from_response(self) -> None:
        result = self.post(
            "Write",
            {
                "file_path": str(self.root / "code" / "example.cpp"),
                "content": "/// Runs the operation.\nvoid Run();\n",
            },
            tool_response={"old_content": "void Run();\n"},
        )
        assert result is not None
        self.assertEqual(result["decision"], "block")

    def test_multiple_edit_payloads_are_checked(self) -> None:
        result = self.post(
            "Edit",
            {
                "file_path": str(self.root / "code" / "example.cpp"),
                "edits": [
                    {
                        "old_string": "void Run();\n",
                        "new_string": "// This patch changes Run.\nvoid Run();\n",
                    },
                    {
                        "old_string": "int value = 1;\n",
                        "new_string": "int value = 2;\n",
                    },
                ],
            },
        )
        assert result is not None
        self.assertEqual(result["decision"], "block")

    def test_apply_patch_extracts_multiple_files_and_context(self) -> None:
        command = """*** Begin Patch
*** Update File: README.md
@@
-Old text.
+New text.
*** Update File: code/example.cpp
@@
 entry.Refresh();
+// Preserve order for wire compatibility.
*** End Patch"""
        edits = STYLE.parse_apply_patch(command, self.root, str(self.root))
        self.assertEqual([edit.path.name for edit in edits], ["README.md", "example.cpp"])
        result = self.post("apply_patch", {"command": command})
        assert result is not None
        context = result["hookSpecificOutput"]["additionalContext"]
        self.assertIn("## Writing prose", context)
        self.assertIn("## Comments", context)
        self.assertNotIn("decision", result)

    def test_apply_patch_comment_violation_blocks(self) -> None:
        command = """*** Begin Patch
*** Update File: code/example.cpp
@@
 void Run();
+/// Runs the operation.
*** End Patch"""
        result = self.post("apply_patch", {"command": command})
        assert result is not None
        self.assertEqual(result["decision"], "block")

    def test_add_file_patch_without_hunk_marker_is_parsed(self) -> None:
        command = """*** Begin Patch
*** Add File: docs/new.md
+# New page
+
+Short text.
*** End Patch"""
        edits = STYLE.parse_apply_patch(command, self.root, str(self.root))
        self.assertEqual(len(edits), 1)
        self.assertEqual(edits[0].pairs[0].old, "")
        self.assertIn("# New page", edits[0].pairs[0].new)

    def test_path_outside_repository_is_ignored(self) -> None:
        result = self.post(
            "Edit",
            {
                "file_path": str(self.root.parent / "outside.md"),
                "old_string": "Old.\n",
                "new_string": "New.\n",
            },
        )
        self.assertIsNone(result)

    def test_subagent_start_gets_both_rule_sections(self) -> None:
        result = STYLE.handle_payload(
            {"hook_event_name": "SubagentStart", "agent_type": "general"},
            self.root,
        )
        assert result is not None
        self.assertEqual(
            result["hookSpecificOutput"]["hookEventName"],
            "SubagentStart",
        )
        context = result["hookSpecificOutput"]["additionalContext"]
        self.assertIn("## Writing prose", context)
        self.assertIn("## Comments", context)

    def test_compact_session_gets_both_rule_sections(self) -> None:
        result = STYLE.handle_payload(
            {"hook_event_name": "SessionStart", "source": "compact"},
            self.root,
        )
        assert result is not None
        self.assertEqual(
            result["hookSpecificOutput"]["hookEventName"],
            "SessionStart",
        )
        self.assertIn(
            "## Comments",
            result["hookSpecificOutput"]["additionalContext"],
        )

    def test_missing_section_becomes_nonblocking_runtime_warning(self) -> None:
        (self.root / "AGENTS.md").write_text("# Root\n", encoding="utf-8")
        context = STYLE.prose_context(self.root, [self.root / "README.md"])
        self.assertIn("Style reminder unavailable", context)
        self.assertIn("fail-open", context)


class RepositoryIntegrationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.root = Path(__file__).resolve().parents[2]

    def test_strict_rule_check_passes(self) -> None:
        completed = subprocess.run(
            [sys.executable, str(SCRIPT), "--check"],
            cwd=self.root,
            capture_output=True,
            text=True,
            check=False,
        )
        self.assertEqual(completed.returncode, 0, completed.stderr)

    def test_claude_uses_shared_script(self) -> None:
        settings = json.loads((self.root / ".claude" / "settings.json").read_text())
        entry = settings["hooks"]["PostToolUse"][0]
        self.assertEqual(entry["matcher"], "Edit|Write")
        command = entry["hooks"][0]["command"]
        self.assertIn(".agents/hooks/style-rules.py", command.replace("\\", "/"))
        self.assertFalse((self.root / ".claude" / "hooks" / "comment-rules.py").exists())

    def test_codex_candidate_uses_supported_events_and_shared_script(self) -> None:
        path = self.root / ".agents" / "hooks" / "codex-hooks.candidate.json"
        config = json.loads(path.read_text(encoding="utf-8"))
        hooks = config["hooks"]
        self.assertEqual(set(hooks), {"PostToolUse", "SubagentStart", "SessionStart"})
        self.assertEqual(hooks["PostToolUse"][0]["matcher"], "Edit|Write")
        self.assertEqual(hooks["SessionStart"][0]["matcher"], "compact")
        for groups in hooks.values():
            for group in groups:
                for handler in group["hooks"]:
                    self.assertIn(".agents/hooks/style-rules.py", handler["command"])
                    self.assertEqual(handler["additionalContextLimit"], 6000)


if __name__ == "__main__":
    unittest.main()
