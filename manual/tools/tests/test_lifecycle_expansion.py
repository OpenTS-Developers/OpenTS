from pathlib import Path
import sys
import tempfile
import unittest


TOOLS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOLS))

import catalog_validation
import schema_validation
import validate_manual
import versioning


class BreakingChangeTests(unittest.TestCase):
    @staticmethod
    def change(**overrides):
        return {
            "title": "Compatibility change",
            "category": "feature",
            "release": "1.0.0",
            "targets": [],
            "credit": ["Programmer"],
            **overrides,
        }

    def test_schema_requires_migration_only_for_breaking_changes(self):
        valid = self.change(
            breaking=True,
            migration=["Replace the removed setting with NewSetting=."],
        )
        self.assertEqual(
            schema_validation.errors_for(
                valid, "authored-change.schema.json", "valid change"),
            [],
        )

        missing = schema_validation.errors_for(
            self.change(breaking=True),
            "authored-change.schema.json",
            "missing migration",
        )
        self.assertTrue(any("migration" in error for error in missing))

        unexpected = schema_validation.errors_for(
            self.change(migration=["This must not be accepted."]),
            "authored-change.schema.json",
            "non-breaking migration",
        )
        self.assertTrue(any("migration" in error for error in unexpected))

    def test_python_validation_and_released_migration_immutability(self):
        registry = {
            "development": "2.0.0",
            "by_version": {
                "1.0.0": {"version": "1.0.0", "status": "released"},
                "2.0.0": {"version": "2.0.0", "status": "development"},
            },
        }
        base_registry = {
            "development": "2.0.0",
            "by_version": registry["by_version"],
        }
        with tempfile.TemporaryDirectory() as temporary:
            manual = Path(temporary)
            changes = manual / "changes"
            changes.mkdir()
            path = changes / "compatibility-change.md"

            path.write_text(
                "---\n"
                "title: Compatibility change\n"
                "category: feature\n"
                "release: 1.0.0\n"
                "breaking: true\n"
                "targets: []\n"
                "credit: [Programmer]\n"
                "---\n",
                encoding="utf-8",
            )
            errors = []
            versioning.validate_changes(
                errors, manual, registry, {}, {}, {}, [],
            )
            self.assertTrue(any(
                "breaking changes require a non-empty migration array" in error
                for error in errors
            ))

            path.write_text(
                "---\n"
                "title: Compatibility change\n"
                "category: feature\n"
                "release: 1.0.0\n"
                "breaking: true\n"
                "migration:\n"
                "  - Use the replacement workflow.\n"
                "targets: []\n"
                "credit: [Programmer]\n"
                "---\n",
                encoding="utf-8",
            )
            errors = []
            versioning.validate_changes(
                errors,
                manual,
                registry,
                {},
                {},
                {},
                [],
                base_changes={
                    "compatibility-change": self.change(
                        breaking=True,
                        migration=["Keep the original workflow."],
                    ),
                },
                base_registry=base_registry,
            )
            self.assertTrue(any(
                "released lifecycle field migration is immutable" in error
                for error in errors
            ))
    def test_schema_and_validation_require_an_author(self):
        without_author = self.change()
        without_author.pop("credit")
        missing = schema_validation.errors_for(
            without_author, "authored-change.schema.json", "missing credit")
        self.assertTrue(any("credit" in error for error in missing))

        empty = schema_validation.errors_for(
            self.change(credit=[]),
            "authored-change.schema.json",
            "empty credit",
        )
        self.assertTrue(any("credit" in error for error in empty))

        registry = {
            "development": "1.0.0",
            "by_version": {
                "1.0.0": {"version": "1.0.0", "status": "development"},
            },
        }
        with tempfile.TemporaryDirectory() as temporary:
            manual = Path(temporary)
            changes = manual / "changes"
            changes.mkdir()
            (changes / "unattributed.md").write_text(
                "---\n"
                "title: Unattributed change\n"
                "category: fix\n"
                "release: 1.0.0\n"
                "targets: []\n"
                "---\n",
                encoding="utf-8",
            )
            errors = []
            versioning.validate_changes(
                errors, manual, registry, {}, {}, {}, [])
        self.assertTrue(any(
            "credit must name at least one author" in error
            for error in errors))

    def test_change_id_cannot_equal_release_version(self):
        registry = {
            "development": "1.0.0",
            "by_version": {
                "1.0.0": {"version": "1.0.0", "status": "development"},
            },
        }
        with tempfile.TemporaryDirectory() as temporary:
            manual = Path(temporary)
            changes = manual / "changes"
            changes.mkdir()
            (changes / "1.0.0.md").write_text(
                "---\n"
                "title: Route collision\n"
                "category: internal\n"
                "release: 1.0.0\n"
                "targets: []\n"
                "credit: [Programmer]\n"
                "---\n",
                encoding="utf-8",
            )
            errors = []
            versioning.validate_changes(
                errors, manual, registry, {}, {}, {}, [])

        self.assertTrue(any(
            "change ID collides with release upgrade route /changes/1.0.0/"
            in error for error in errors))




class LifecycleEntityExpansionTests(unittest.TestCase):
    @staticmethod
    def empty_key_delta():
        return {"added": set(), "changed": set(), "removed": set()}

    @staticmethod
    def empty_scripting_delta():
        return {
            plural: {
                "added": set(),
                "changed": set(),
                "removed": set(),
                "shifted": {},
            }
            for plural in versioning.SCRIPTING_TYPES
        }

    @staticmethod
    def command(identifier, route_id, description, declaring_class="OneCommandClass"):
        return {
            "id": identifier,
            "route_id": route_id,
            "kind": "registered",
            "title": identifier,
            "description": description,
            "availability": {"builds": ["release", "debug"]},
            "_provenance": {"source": "code/init.cpp", "class": declaring_class},
        }

    def test_system_and_command_targets_tombstones_and_replacements_resolve(self):
        entities = {
            "system": {
                "new-system": {"route": "/systems/new-system/"},
            },
            "command": {
                "NewCommand": {"route_id": "newcommand"},
            },
        }
        with tempfile.TemporaryDirectory() as temporary:
            manual = Path(temporary)
            (manual / "changes").mkdir()
            (manual / "data").mkdir()
            (manual / "changes" / "new-entities.md").write_text(
                "---\n"
                "title: New public entities\n"
                "category: feature\n"
                "release: 1.0.0\n"
                "targets:\n"
                "  - type: system\n"
                "    id: new-system\n"
                "    effect: added\n"
                "  - type: command\n"
                "    id: NewCommand\n"
                "    effect: added\n"
                "credit: [Programmer]\n"
                "---\n",
                encoding="utf-8",
            )
            registry = {
                "development": "1.0.0",
                "by_version": {
                    "1.0.0": {"version": "1.0.0", "status": "development"},
                },
            }
            errors = []
            versioning.validate_changes(
                errors, manual, registry, {}, {}, {}, [], entities=entities)
            self.assertEqual(errors, [])

            (manual / "data" / "tombstones.yaml").write_text(
                "- type: system\n"
                "  id: old-system\n"
                "  route: /systems/old-system/\n"
                "  search_aliases: []\n"
                "  summary: Replaced system.\n"
                "  replacement:\n"
                "    type: command\n"
                "    id: NewCommand\n"
                "- type: command\n"
                "  id: OldCommand\n"
                "  route: /commands/oldcommand/\n"
                "  search_aliases: []\n"
                "  summary: Replaced command.\n"
                "  replacement:\n"
                "    type: system\n"
                "    id: new-system\n",
                encoding="utf-8",
            )
            errors = []
            versioning.validate_tombstones(
                errors, manual, {}, {}, {}, {}, entities)
            self.assertEqual(errors, [])

    def test_history_may_cite_an_entity_removed_later(self):
        with tempfile.TemporaryDirectory() as temporary:
            manual = Path(temporary)
            (manual / "changes").mkdir()
            (manual / "changes" / "old-behavior.md").write_text(
                "---\n"
                "title: Change the old command\n"
                "category: feature\n"
                "release: 1.0.0\n"
                "targets:\n"
                "  - type: command\n"
                "    id: OldCommand\n"
                "    effect: changed\n"
                "credit: [Programmer]\n"
                "---\n",
                encoding="utf-8",
            )
            (manual / "changes" / "old-removal.md").write_text(
                "---\n"
                "title: Remove the old command\n"
                "category: feature\n"
                "release: 2.0.0\n"
                "targets:\n"
                "  - type: command\n"
                "    id: OldCommand\n"
                "    effect: removed\n"
                "credit: [Programmer]\n"
                "---\n",
                encoding="utf-8",
            )
            registry = {
                "development": "2.0.0",
                "by_version": {
                    "1.0.0": {"version": "1.0.0", "status": "released"},
                    "2.0.0": {"version": "2.0.0", "status": "development"},
                },
            }
            tombstones = [{
                "type": "command",
                "id": "OldCommand",
                "route": "/commands/oldcommand/",
                "search_aliases": [],
                "summary": "Removed command.",
            }]
            errors = []
            versioning.validate_changes(
                errors, manual, registry, {}, {}, {}, tombstones,
                entities={"command": {}})
            self.assertEqual(errors, [])

            # A tombstone answers only for the entity itself, never for a scope of one.
            (manual / "changes" / "old-behavior.md").write_text(
                "---\n"
                "title: Change the old key\n"
                "category: feature\n"
                "release: 1.0.0\n"
                "targets:\n"
                "  - type: key\n"
                "    id: OldKey\n"
                "    effect: changed\n"
                "    scope: campaign\n"
                "credit: [Programmer]\n"
                "---\n",
                encoding="utf-8",
            )
            tombstones.append({
                "type": "key",
                "id": "OldKey",
                "route": "/keys/oldkey/",
                "search_aliases": [],
                "summary": "Removed key.",
            })
            errors = []
            versioning.validate_changes(
                errors, manual, registry, {}, {}, {}, tombstones,
                entities={"command": {}})
            self.assertTrue(
                any("unknown active entity" in error for error in errors))

    def test_command_deltas_ignore_provenance_and_require_lifecycle(self):
        base = {
            "registered_commands": [
                self.command("Same", "same", "Same", "SameCommandClass"),
                self.command("Changed", "changed", "Before", "ChangedCommandClass"),
                self.command("Removed", "removed", "Removed", "RemovedCommandClass"),
                self.command("Route", "old-route", "Route", "RouteCommandClass"),
            ],
            "fixed_controls": [],
            "launch_options": [],
        }
        current = {
            "registered_commands": [
                self.command("Same", "same", "Same", "MovedCommandClass"),
                self.command("Changed", "changed", "After", "ChangedCommandClass"),
                self.command("Added", "added", "Added", "AddedCommandClass"),
                self.command("Route", "new-route", "Route", "RouteCommandClass"),
            ],
            "fixed_controls": [],
            "launch_options": [],
        }
        # Titles are presentation naming and never require lifecycle records.
        current["registered_commands"][0]["title"] = "Renamed"
        delta = validate_manual.classify_command_deltas(current, base)
        self.assertEqual(delta["added"], {"Added"})
        self.assertEqual(delta["changed"], {"Changed"})
        self.assertEqual(delta["removed"], {"Removed"})
        self.assertEqual(delta["route_changed"], {"Route"})

        changes = {
            "commands": {
                "data": {
                    "targets": [
                        {"type": "command", "id": "Added", "scope": None,
                         "effect": "added"},
                        {"type": "command", "id": "Changed", "scope": None,
                         "effect": "changed"},
                        {"type": "command", "id": "Removed", "scope": None,
                         "effect": "removed"},
                    ],
                },
            },
        }
        tombstones = [{
            "type": "command",
            "id": "Removed",
            "route": "/commands/removed/",
        }]
        errors = []
        versioning.validate_branch_lifecycle(
            errors, {}, {}, {}, {}, {}, {}, changes, tombstones,
            self.empty_key_delta(), self.empty_scripting_delta(),
            command_delta=delta, base_commands=base)
        self.assertEqual(errors, [])

        errors = []
        versioning.validate_branch_lifecycle(
            errors, {}, {}, {}, {}, {}, {}, {}, [],
            self.empty_key_delta(), self.empty_scripting_delta(),
            command_delta=None, base_commands=None)
        self.assertEqual(errors, [])

        tombstones[0]["route"] = "/commands/not-the-stable-route/"
        errors = []
        versioning.validate_branch_lifecycle(
            errors, {}, {}, {}, {}, {}, {}, changes, tombstones,
            self.empty_key_delta(), self.empty_scripting_delta(),
            command_delta=delta, base_commands=base)
        self.assertTrue(any(
            "command:Removed: tombstone route must remain '/commands/removed/'"
            in error for error in errors))

    def test_format_tombstones_preserve_compatibility_and_default_routes(self):
        base_formats = {
            "teamtypes": {"route": "/mapping/team-types/"},
            "mix": {"route": "/formats/mix/"},
        }
        changes = {
            "removed-formats": {
                "data": {
                    "targets": [
                        {"type": "format", "id": "teamtypes", "scope": None,
                         "effect": "removed"},
                        {"type": "format", "id": "mix", "scope": None,
                         "effect": "removed"},
                    ],
                },
            },
        }
        tombstones = [
            {"type": "format", "id": "teamtypes", "route": "/formats/teamtypes/"},
            {"type": "format", "id": "mix", "route": "/mapping/mix/"},
        ]
        errors = []
        versioning.validate_branch_lifecycle(
            errors, {}, {}, {}, {}, {}, base_formats, changes, tombstones,
            self.empty_key_delta(), self.empty_scripting_delta())
        self.assertTrue(any(
            "format:teamtypes: tombstone route must remain '/mapping/team-types/'"
            in error for error in errors))
        self.assertTrue(any(
            "format:mix: tombstone route must remain '/formats/mix/'"
            in error for error in errors))

        self.assertEqual(
            versioning.entity_route("format", "teamtypes"),
            "/mapping/team-types/",
        )
        self.assertEqual(
            versioning.entity_route("format", "mix"),
            "/formats/mix/",
        )

    def test_system_and_command_lifecycle_errors_are_actionable(self):
        for entity_type, identifier in (
                ("system", "new-system"), ("command", "NewCommand")):
            result = validate_manual._actionable(
                f"{entity_type}:{identifier}: add a change target with effect: added")
            self.assertIn(f"--target-type {entity_type}", result)
            self.assertIn(f"--target-id {identifier}", result)

    def test_related_references_reject_unknown_targets_and_duplicates(self):
        duplicate = {
            "title": "Duplicate relations",
            "summary": "Exercises relation identity validation.",
            "category": "ai-teams",
            "keys": [],
            "related": [
                {"type": "format", "id": "scripts"},
                {"type": "format", "id": "scripts"},
            ],
        }
        schema_errors = schema_validation.errors_for(
            duplicate, "authored-system.schema.json", "duplicate relations")
        self.assertTrue(any("unique" in error for error in schema_errors))

        with tempfile.TemporaryDirectory() as temporary:
            manual = Path(temporary)
            formats = manual / "content" / "formats"
            formats.mkdir(parents=True)
            (formats / "broken.md").write_text(
                "---\n"
                "format_id: broken\n"
                "related:\n"
                "  - { type: internal, id: missing-internal }\n"
                "---\n",
                encoding="utf-8",
            )
            errors = []
            catalog_validation.validate_relations(
                errors, manual, {}, {}, {}, [], {}, {})

        self.assertTrue(any(
            "unknown entity internal:missing-internal" in error
            for error in errors))


if __name__ == "__main__":
    unittest.main()
