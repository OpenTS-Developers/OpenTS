import copy
from contextlib import redirect_stdout
import io
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock

TOOLS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOLS))

import manage
import schema_validation
import versioning
import validate_manual


def scope(value_type="integer", candidate=None, applies_to="UnitType"):
    return {
        "applies_to": [applies_to],
        "file": "rules.ini",
        "section": {"kind": "identifier", "source": "object-type"},
        "value_type": value_type,
        "_provenance": {"default_candidate": candidate},
    }


def entry(*scopes):
    return {"scopes": list(scopes)}


class DeltaTests(unittest.TestCase):
    def test_key_delta_ignores_candidates_and_detects_semantic_changes(self):
        base = {
            "Same": entry(scope(candidate="1")),
            "Changed": entry(scope()),
            "Removed": entry(scope()),
            "Expanded": entry(scope()),
        }
        current = {
            "Same": entry(scope(candidate="2")),
            "Changed": entry(scope("floating point")),
            "Expanded": entry(scope(), scope(applies_to="AircraftType")),
            "Added": entry(scope()),
        }

        delta = validate_manual.classify_key_deltas(current, base)

        self.assertEqual(delta["added"], {"Added"})
        self.assertEqual(delta["changed"], {"Changed", "Expanded"})
        self.assertEqual(delta["removed"], {"Removed"})

    def test_added_and_removed_entities_require_lifecycle_targets(self):
        base = {
            "Removed": entry(scope()),
            "RenamedOld": entry(scope()),
        }
        current = {
            "Added": entry(scope()),
            "RenamedNew": entry(scope()),
        }
        changes = {
            "feature": {
                "data": {
                    "targets": [
                        {"type": "key", "id": "Added", "scope": None, "effect": "added"},
                        {"type": "key", "id": "RenamedNew", "scope": None, "effect": "added"},
                        {"type": "key", "id": "Removed", "scope": None, "effect": "removed"},
                        {"type": "key", "id": "RenamedOld", "scope": None, "effect": "removed"},
                    ]
                }
            }
        }
        tombstones = [
            {
                "type": "key",
                "id": "Removed",
                "route": "/keys/removed/",
            },
            {
                "type": "key",
                "id": "RenamedOld",
                "route": "/keys/renamedold/",
            },
        ]
        empty_scripting = {
            kind: {"added": set(), "changed": set(), "removed": set(), "shifted": {}}
            for kind in validate_manual.SCRIPTING_TABLES
        }
        errors = []

        delta = validate_manual.classify_key_deltas(current, base)
        versioning.validate_branch_lifecycle(
            errors,
            current,
            base,
            {},
            {},
            changes,
            tombstones,
            delta,
            empty_scripting,
        )

        self.assertFalse(errors)
        self.assertEqual(delta["added"], {"Added", "RenamedNew"})
        self.assertEqual(delta["removed"], {"Removed", "RenamedOld"})

    def test_scope_addition_and_wrong_tombstone_route_fail(self):
        base = {
            "Changed": entry(scope()),
            "Removed": entry(scope()),
        }
        current = {
            "Changed": entry(scope(), scope(applies_to="AircraftType")),
        }
        tombstones = [{
            "type": "key",
            "id": "Removed",
            "route": "/keys/not-the-stable-route/",
        }]
        empty_scripting = {
            kind: {"added": set(), "changed": set(), "removed": set(), "shifted": {}}
            for kind in validate_manual.SCRIPTING_TABLES
        }
        errors = []

        delta = validate_manual.classify_key_deltas(current, base)
        versioning.validate_branch_lifecycle(
            errors,
            current,
            base,
            {},
            {},
            {},
            tombstones,
            delta,
            empty_scripting,
        )

        self.assertTrue(any("Changed[aircrafttype]" in error for error in errors))
        self.assertTrue(any("must remain '/keys/removed/'" in error for error in errors))
        self.assertTrue(any("effect: removed" in error for error in errors))

    def test_full_semver_ordering_and_build_metadata_rejection(self):
        self.assertGreater(versioning.Version("1.10.0"), versioning.Version("1.9.0"))
        self.assertGreater(versioning.Version("1.0.0"), versioning.Version("1.0.0-rc.1"))
        self.assertGreater(
            versioning.Version("1.0.0-beta.11"),
            versioning.Version("1.0.0-beta.2"),
        )
        errors = []
        self.assertIsNone(versioning.semver("1.0.0+build.1", "test", errors))
        self.assertTrue(any("build metadata" in error for error in errors))
        self.assertIsNone(versioning.semver("1.0", "test", errors))
        self.assertTrue(any("invalid SemVer" in error for error in errors))

    def test_release_registry_status_dates_duplicates_and_cmake_sync(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            data = root / "manual" / "data"
            data.mkdir(parents=True)
            releases = data / "releases.yaml"
            cmake = root / "CMakeLists.txt"

            releases.write_text(
                "releases:\n"
                "  - version: 0.9.0\n"
                "    status: released\n"
                "    date: '2026-01-01'\n"
                "  - version: 1.0.0-rc.1\n"
                "    status: development\n",
                encoding="utf-8",
            )
            cmake.write_text(
                "project(OpenTS VERSION 1.0.0 LANGUAGES CXX)\n"
                'set(OPENTS_VERSION_PRERELEASE "rc.1")\n',
                encoding="utf-8",
            )
            errors = []
            registry = versioning.validate_releases(
                errors, root / "manual", root)

            self.assertFalse(errors)
            self.assertEqual(registry["development"], "1.0.0-rc.1")

            releases.write_text(
                "releases:\n"
                "  - version: 0.9.0\n"
                "    status: released\n"
                "  - version: 1.0.0-rc.1\n"
                "    status: development\n"
                "    date: '2026-02-01'\n",
                encoding="utf-8",
            )
            cmake.write_text(
                "project(OpenTS VERSION 0.9.0 LANGUAGES CXX)\n"
                'set(OPENTS_VERSION_PRERELEASE "rc.1")\n',
                encoding="utf-8",
            )
            errors = []
            versioning.validate_releases(errors, root / "manual", root)
            self.assertTrue(any("must match the development SemVer core" in error
                                for error in errors))
            schema_errors = schema_validation.errors_for(
                versioning.read_yaml(releases), "releases.schema.json", "releases")
            self.assertTrue(any("date" in error for error in schema_errors))

            releases.write_text(
                "releases:\n"
                "  - version: 1.0.0\n"
                "    status: released\n"
                "    date: '2026-02-01'\n"
                "  - version: 1.0.0\n"
                "    status: development\n",
                encoding="utf-8",
            )
            cmake.write_text(
                "project(OpenTS VERSION 1.0.0 LANGUAGES CXX)\n"
                'set(OPENTS_VERSION_PRERELEASE "")\n',
                encoding="utf-8",
            )
            errors = []
            versioning.validate_releases(errors, root / "manual", root)
            self.assertTrue(any("duplicate version 1.0.0" in error
                                for error in errors))

    def test_cmake_prerelease_label_tracks_the_development_version(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            data = root / "manual" / "data"
            data.mkdir(parents=True)
            releases = data / "releases.yaml"
            cmake = root / "CMakeLists.txt"

            releases.write_text(
                "releases:\n"
                "  - version: 0.2.0-beta1\n"
                "    status: development\n",
                encoding="utf-8",
            )

            # A label that disagrees with the registry is rejected.
            cmake.write_text(
                "project(OpenTS VERSION 0.2.0 LANGUAGES CXX)\n"
                'set(OPENTS_VERSION_PRERELEASE "alpha1")\n',
                encoding="utf-8",
            )
            errors = []
            versioning.validate_releases(errors, root / "manual", root)
            self.assertTrue(any("must match the development version's label" in error
                                for error in errors))

            # So is a missing declaration.
            cmake.write_text(
                "project(OpenTS VERSION 0.2.0 LANGUAGES CXX)\n",
                encoding="utf-8",
            )
            errors = []
            versioning.validate_releases(errors, root / "manual", root)
            self.assertTrue(any("OPENTS_VERSION_PRERELEASE" in error
                                for error in errors))

            # The matching label passes.
            cmake.write_text(
                "project(OpenTS VERSION 0.2.0 LANGUAGES CXX)\n"
                'set(OPENTS_VERSION_PRERELEASE "beta1")\n',
                encoding="utf-8",
            )
            errors = []
            versioning.validate_releases(errors, root / "manual", root)
            self.assertFalse(errors)

            # A stable development version requires an empty label.
            releases.write_text(
                "releases:\n"
                "  - version: 0.2.0\n"
                "    status: development\n",
                encoding="utf-8",
            )
            errors = []
            versioning.validate_releases(errors, root / "manual", root)
            self.assertTrue(any("must match the development version's label" in error
                                for error in errors))

    def test_new_numeric_aliases_reserve_only_current_unused_indices(self):
        scripting = {
            "trigger_actions": [
                {"id": "TACTION_A", "index": 0},
                {"id": "TACTION_B", "index": 2},
            ],
            "trigger_events": [],
            "team_missions": [],
        }
        base_aliases = {
            "actions": {"0": "TACTION_A"},
            "events": {},
            "missions": {},
        }
        with tempfile.TemporaryDirectory() as temporary:
            manual = Path(temporary)
            data = manual / "data"
            data.mkdir()

            aliases = data / "scripting-route-aliases.yaml"
            aliases.write_text(
                "actions:\n"
                "  '0': TACTION_A\n"
                "events: {}\n"
                "missions: {}\n",
                encoding="utf-8",
            )
            errors = []
            versioning.validate_aliases(
                errors, manual, scripting, [], base_aliases)
            self.assertTrue(any(
                "reserve unused numeric route actions/2" in error
                for error in errors))

            aliases.write_text(
                "actions:\n"
                "  '0': TACTION_A\n"
                "  '2': TACTION_A\n"
                "events: {}\n"
                "missions: {}\n",
                encoding="utf-8",
            )
            errors = []
            versioning.validate_aliases(
                errors, manual, scripting, [], base_aliases)
            self.assertTrue(any(
                "new scripting alias actions/2 must use" in error
                for error in errors))

            aliases.write_text(
                "actions:\n"
                "  '0': TACTION_A\n"
                "  '2': TACTION_B\n"
                "events: {}\n"
                "missions: {}\n",
                encoding="utf-8",
            )
            errors = []
            versioning.validate_aliases(
                errors, manual, scripting, [], base_aliases)
            self.assertFalse(errors)
    def test_release_publication_and_change_identity_are_immutable(self):
        base_registry = {
            "development": "1.0.0",
            "by_version": {
                "0.9.0": {
                    "version": "0.9.0",
                    "status": "released",
                    "date": "2026-01-01",
                },
                "1.0.0": {
                    "version": "1.0.0",
                    "status": "development",
                },
            },
        }
        current_registry = {
            "development": "1.1.0",
            "by_version": {
                "0.9.0": {
                    "version": "0.9.0",
                    "status": "released",
                    "date": "2026-01-02",
                },
                "1.1.0": {
                    "version": "1.1.0",
                    "status": "development",
                },
            },
        }
        errors = []
        versioning.validate_released_registry_history(
            errors, current_registry, base_registry)

        self.assertTrue(any("date is immutable" in error for error in errors))
        self.assertTrue(any(
            "development version 1.0.0 must remain" in error
            for error in errors))

        with tempfile.TemporaryDirectory() as temporary:
            manual = Path(temporary)
            changes_dir = manual / "changes"
            changes_dir.mkdir()
            (changes_dir / "restamped.md").write_text(
                "---\n"
                "title: Restamped\n"
                "category: feature\n"
                "release: 1.1.0\n"
                "targets: []\n"
                "credit: [Author]\n"
                "---\n",
                encoding="utf-8",
            )
            (changes_dir / "frozen.md").write_text(
                "---\n"
                "title: Frozen\n"
                "category: fix\n"
                "release: 1.0.0\n"
                "targets: []\n"
                "credit: [Author]\n"
                "---\n",
                encoding="utf-8",
            )
            (changes_dir / "active-scope.md").write_text(
                "---\n"
                "title: Invalid scope removal\n"
                "category: internal\n"
                "release: 1.1.0\n"
                "targets:\n"
                "  - type: key\n"
                "    id: Scoped\n"
                "    scope: unittype\n"
                "    effect: removed\n"
                "credit: [Author]\n"
                "---\n",
                encoding="utf-8",
            )
            registry = {
                "development": "1.1.0",
                "by_version": {
                    "1.0.0": {
                        "version": "1.0.0",
                        "status": "released",
                    },
                    "1.1.0": {
                        "version": "1.1.0",
                        "status": "development",
                    },
                },
            }
            base = copy.deepcopy(registry)
            base_changes = {
                "restamped": {
                    "title": "Restamped",
                    "category": "feature",
                    "release": "1.0.0",
                    "targets": [],
                    "credit": ["Author"],
                },
                "frozen": {
                    "title": "Frozen",
                    "category": "feature",
                    "release": "1.0.0",
                    "targets": [],
                    "credit": ["Author"],
                },
                "removed-change-id": {
                    "title": "Gone",
                    "category": "internal",
                    "release": "1.0.0",
                    "targets": [],
                    "credit": ["Author"],
                },
            }
            errors = []
            versioning.validate_changes(
                errors,
                manual,
                registry,
                {"Scoped": entry(scope())},
                {},
                {},
                [],
                base_changes,
                base,
            )

            self.assertTrue(any(
                "release assignment is stable" in error for error in errors))
            self.assertTrue(any(
                "released lifecycle field category is immutable" in error
                for error in errors))
            self.assertTrue(any(
                "stable change IDs cannot be removed or renamed" in error
                for error in errors))
            self.assertTrue(any(
                "removed key scope is still active" in error
                for error in errors))

    def test_no_lifecycle_event_can_share_or_follow_removal_release(self):
        target = {
            "type": "action",
            "id": "TACTION_OLD",
            "scope": None,
        }
        changes = {
            "removed": {
                "data": {
                    "release": "1.0.0",
                    "targets": [{**target, "effect": "removed"}],
                },
            },
            "also-changed": {
                "data": {
                    "release": "1.0.0",
                    "targets": [{**target, "effect": "changed"}],
                },
            },
        }
        registry = {
            "by_version": {
                "1.0.0": {"_parsed": versioning.Version("1.0.0")},
            },
        }
        errors = []
        versioning.validate_history(
            errors,
            registry,
            changes,
            [{"type": "action", "id": "TACTION_OLD"}],
        )

        self.assertTrue(any(
            "lifecycle event follows its removal" in error
            for error in errors))
    def test_scripting_index_shift_requires_changed_target(self):
        empty_keys = {}
        empty_key_delta = {
            "added": set(),
            "changed": set(),
            "removed": set(),
        }
        scripting_delta = {
            kind: {
                "added": set(),
                "changed": set(),
                "removed": set(),
                "shifted": {},
            }
            for kind in validate_manual.SCRIPTING_TABLES
        }
        scripting_delta["actions"]["shifted"] = {"TACTION_WIN": (1, 2)}
        errors = []

        versioning.validate_branch_lifecycle(
            errors,
            empty_keys,
            empty_keys,
            {},
            {},
            {},
            [],
            empty_key_delta,
            scripting_delta,
        )

        self.assertTrue(any(
            "action:TACTION_WIN" in error and "effect: changed" in error
            for error in errors))

        changes = {
            "reindex-win": {
                "data": {
                    "targets": [{
                        "type": "action",
                        "id": "TACTION_WIN",
                        "scope": None,
                        "effect": "changed",
                    }],
                },
            },
        }
        errors = []
        versioning.validate_branch_lifecycle(
            errors,
            empty_keys,
            empty_keys,
            {},
            {},
            changes,
            [],
            empty_key_delta,
            scripting_delta,
        )
        self.assertFalse(errors)
    def test_scripting_delta_reports_content_and_index_changes(self):
        base = {
            "trigger_actions": [
                {"id": "A", "index": 0, "need": "NEED_NONE"},
                {"id": "B", "index": 1, "need": "NEED_NONE"},
                {
                    "id": "Shift",
                    "index": 2,
                    "need": "NEED_NONE",
                    "ini_example": {"line": "<TriggerID>=1,2,0,0"},
                },
            ],
            "trigger_events": [],
            "team_missions": [],
        }
        current = copy.deepcopy(base)
        current["trigger_actions"] = [
            {"id": "A", "index": 1, "need": "NEED_NUMBER"},
            {"id": "C", "index": 0, "need": "NEED_NONE"},
            {
                "id": "Shift",
                "index": 3,
                "need": "NEED_NONE",
                "ini_example": {"line": "<TriggerID>=1,3,0,0"},
            },
        ]

        delta = validate_manual.classify_scripting_deltas(current, base)["actions"]

        self.assertEqual(delta["added"], {"C"})
        self.assertEqual(delta["changed"], {"A"})
        self.assertEqual(delta["removed"], {"B"})
        self.assertEqual(delta["shifted"], {"A": (0, 1), "Shift": (2, 3)})


class GenerationTests(unittest.TestCase):
    @staticmethod
    def successful_runner(script, arguments, output):
        Path(output).write_text(script.name, encoding="utf-8")

    def test_update_replaces_both_files_after_generation(self):
        with tempfile.TemporaryDirectory() as temporary:
            data = Path(temporary)
            for _, _, name in manage.GENERATOR_SPECS:
                (data / name).write_text("old", encoding="utf-8")

            manage.update_generated(self.successful_runner, data)

            self.assertEqual(
                (data / "ini-keys.yaml").read_text(encoding="utf-8"),
                "extract.py")
            self.assertEqual(
                (data / "scripting.yaml").read_text(encoding="utf-8"),
                "scripting.py")

    def test_generator_failure_leaves_both_targets_untouched(self):
        calls = 0

        def failing_runner(script, arguments, output):
            nonlocal calls
            calls += 1
            if calls == 2:
                raise manage.ManualCommandError("synthetic failure")
            Path(output).write_text("fresh", encoding="utf-8")

        with tempfile.TemporaryDirectory() as temporary:
            data = Path(temporary)
            for _, _, name in manage.GENERATOR_SPECS:
                (data / name).write_text("old", encoding="utf-8")

            with self.assertRaises(manage.ManualCommandError):
                manage.update_generated(failing_runner, data)

            for _, _, name in manage.GENERATOR_SPECS:
                self.assertEqual(
                    (data / name).read_text(encoding="utf-8"), "old")

    def test_os_replace_failure_rolls_back_the_first_target(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            data, fresh = root / "data", root / "fresh"
            data.mkdir()
            fresh.mkdir()
            generated = {}
            for _, _, name in manage.GENERATOR_SPECS:
                (data / name).write_text("old", encoding="utf-8")
                generated[name] = fresh / name
                generated[name].write_text("new", encoding="utf-8")

            real_replace = manage.os.replace
            calls = 0

            def fail_second_replace(source, destination):
                nonlocal calls
                calls += 1
                if calls == 2:
                    raise OSError("synthetic replace failure")
                return real_replace(source, destination)

            with mock.patch.object(
                    manage.os, "replace", side_effect=fail_second_replace):
                with self.assertRaises(OSError):
                    manage.replace_generated(generated, data)

            for _, _, name in manage.GENERATOR_SPECS:
                self.assertEqual(
                    (data / name).read_text(encoding="utf-8"), "old")

    def test_drift_check_is_read_only(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            data, fresh = root / "data", root / "fresh"
            data.mkdir()
            fresh.mkdir()
            target = data / "ini-keys.yaml"
            target.write_text("old\n", encoding="utf-8")
            generated = fresh / "ini-keys.yaml"
            generated.write_text("new\n", encoding="utf-8")

            with redirect_stdout(io.StringIO()):
                errors = manage.drift_errors({"ini-keys.yaml": generated}, data)

            self.assertTrue(errors)
            self.assertEqual(target.read_text(encoding="utf-8"), "old\n")

    def test_explicit_missing_base_revision_fails(self):
        with self.assertRaises(manage.ManualCommandError):
            manage.load_base("refs/heads/definitely-not-a-real-manual-base")


if __name__ == "__main__":
    unittest.main()
