from pathlib import Path
import sys
import unittest

TOOLS = Path(__file__).resolve().parents[1]
ROOT = TOOLS.parents[1]
sys.path.insert(0, str(TOOLS))

import enum_drift
import validate_manual
import versioning


class EnumContractTests(unittest.TestCase):
    def reader(self, path):
        return (ROOT / path).read_text(encoding="utf-8")

    def test_representative_source_adapters_compare_tokens_values_and_order(self):
        action = enum_drift.signature(enum_drift.ADAPTERS["ActionType"], self.reader)
        mission = enum_drift.signature(enum_drift.ADAPTERS["MissionType"], self.reader)
        foundation = enum_drift.signature(enum_drift.ADAPTERS["BSizeType"], self.reader)

        self.assertIn(("ACTION_DROP_POD", 50, "DropPod"), action)
        self.assertEqual(mission[14], ("MISSION_HUNT", 14, "14"))
        self.assertEqual(foundation[9], ("BSIZE_33_REF", 9, "3x3Refinery"))

    def test_source_token_numeric_and_order_changes_alter_signatures(self):
        sources = {
            path: self.reader(path)
            for path in enum_drift.source_paths()
        }
        action_adapter = enum_drift.ADAPTERS["ActionType"]
        original_action = enum_drift.signature(action_adapter, lambda path: sources[path])
        changed_tokens = dict(sources)
        changed_tokens["code/ccini.cpp"] = changed_tokens["code/ccini.cpp"].replace(
            '"Move",', '"Relocate",', 1)
        changed_action = enum_drift.signature(
            action_adapter, lambda path: changed_tokens[path])
        self.assertNotEqual(original_action, changed_action)

        mission_adapter = enum_drift.ADAPTERS["MissionType"]
        original_mission = enum_drift.signature(mission_adapter, lambda path: sources[path])
        changed_numbers = dict(sources)
        changed_numbers["code/mission.hh"] = changed_numbers["code/mission.hh"].replace(
            "MISSION_ATTACK,", "MISSION_ATTACK=4,", 1)
        changed_mission = enum_drift.signature(
            mission_adapter, lambda path: changed_numbers[path])
        self.assertNotEqual(original_mission, changed_mission)
        self.assertNotEqual(
            [constant for constant, _, _ in original_mission],
            [constant for constant, _, _ in tuple(reversed(original_mission))],
        )

    def test_dynamic_identifier_catalogs_are_not_enum_bindings(self):
        self.assertTrue(enum_drift.is_dynamic_binding(
            "key_value_types", "HouseType"))
        self.assertTrue(enum_drift.is_dynamic_binding(
            "key_value_types", "list of sounds"))
        self.assertTrue(enum_drift.is_dynamic_binding(
            "scripting_parameter_types", "weapon"))
        self.assertFalse(enum_drift.is_dynamic_binding(
            "key_value_types", "ArmorType"))
        self.assertFalse(enum_drift.is_dynamic_binding(
            "scripting_parameter_types", "mission"))

    def test_branch_classifier_distinguishes_metadata_from_source_lifecycle(self):
        signature = enum_drift.signature(enum_drift.ADAPTERS["MissionType"], self.reader)
        current = {
            "MissionType": {"slug": "mission", "_signature": signature},
        }
        metadata_only = validate_manual.classify_enum_deltas(
            current, {}, {"MissionType": signature})
        self.assertFalse(metadata_only["added"])
        self.assertFalse(metadata_only["changed"])

        introduced = validate_manual.classify_enum_deltas(current, {}, {})
        self.assertEqual(introduced["added"], {"MissionType"})

        changed = validate_manual.classify_enum_deltas(
            current, {"MissionType": {"slug": "mission"}},
            {"MissionType": (("MISSION_SLEEP", 99, "99"),)})
        self.assertEqual(changed["changed"], {"MissionType"})

        removed = validate_manual.classify_enum_deltas(
            {}, {"MissionType": {"slug": "mission"}}, {})
        self.assertEqual(removed["removed"], {"MissionType"})

    def test_enum_lifecycle_tracks_source_drift_only(self):
        """Enum pages are a curated docs selection: only signature drift on a
        selected domain is an engine lifecycle event."""
        empty_keys = {}
        empty_key_delta = {"added": set(), "changed": set(), "removed": set()}
        empty_scripting = {
            kind: {"added": set(), "changed": set(), "removed": set(), "shifted": {}}
            for kind in validate_manual.SCRIPTING_TABLES
        }
        enum_delta = {
            "added": {"NewEnum"},
            "changed": {"ChangedEnum"},
            "removed": {"OldEnum"},
        }
        errors = []
        versioning.validate_branch_lifecycle(
            errors,
            empty_keys, empty_keys,
            {}, {}, {}, [],
            empty_key_delta, empty_scripting,
            enum_delta,
        )
        self.assertTrue(any("enum:ChangedEnum" in error and "effect: changed" in error for error in errors))
        self.assertFalse(any("enum:NewEnum" in error for error in errors))
        self.assertFalse(any("enum:OldEnum" in error for error in errors))
        self.assertFalse(any("tombstone" in error for error in errors))

        changes = {
            "enum-lifecycle": {
                "data": {"targets": [
                    {"type": "enum", "id": "ChangedEnum", "scope": None, "effect": "changed"},
                ]},
            },
        }
        errors = []
        versioning.validate_branch_lifecycle(
            errors,
            empty_keys, empty_keys,
            {}, {}, changes, [],
            empty_key_delta, empty_scripting,
            enum_delta,
        )
        self.assertFalse(errors)


if __name__ == "__main__":
    unittest.main()