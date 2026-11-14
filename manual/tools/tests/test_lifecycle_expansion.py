from copy import deepcopy
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

    def test_schema_allows_breaking_without_migration(self):
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
        self.assertEqual(missing, [])

        empty = schema_validation.errors_for(
            self.change(breaking=True, migration=[]),
            "authored-change.schema.json",
            "empty migration",
        )
        self.assertTrue(any("migration" in error for error in empty))

        unexpected = schema_validation.errors_for(
            self.change(migration=["This must not be accepted."]),
            "authored-change.schema.json",
            "non-breaking migration",
        )
        self.assertTrue(any("migration" in error for error in unexpected))

    def test_python_validation_and_released_editorial_corrections(self):
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
            self.assertEqual(errors, [])

            base_change = self.change(
                breaking=True, migration=["Keep the original workflow."],
            )
            for field, value in (
                ("category", "fix"),
                ("targets", [{"type": "system", "id": "old-system", "effect": "changed"}]),
                ("breaking", False),
            ):
                with self.subTest(field=field):
                    errors = []
                    versioning.validate_changes(
                        errors, manual, registry, {}, {}, {}, [],
                        base_changes={"compatibility-change": {**base_change, field: value}},
                        base_registry=base_registry,
                    )
                    self.assertTrue(any(
                        f"released lifecycle field {field} is immutable" in error
                        for error in errors
                    ))

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
                base_changes={"compatibility-change": base_change},
                base_registry=base_registry,
            )
            self.assertEqual(errors, [])
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
            errors, {}, {}, {}, {}, changes, tombstones,
            self.empty_key_delta(), self.empty_scripting_delta(),
            command_delta=delta, base_commands=base)
        self.assertEqual(errors, [])

        errors = []
        versioning.validate_branch_lifecycle(
            errors, {}, {}, {}, {}, {}, [],
            self.empty_key_delta(), self.empty_scripting_delta(),
            command_delta=None, base_commands=None)
        self.assertEqual(errors, [])

        tombstones[0]["route"] = "/commands/not-the-stable-route/"
        errors = []
        versioning.validate_branch_lifecycle(
            errors, {}, {}, {}, {}, changes, tombstones,
            self.empty_key_delta(), self.empty_scripting_delta(),
            command_delta=delta, base_commands=base)
        self.assertTrue(any(
            "command:Removed: tombstone route must remain '/commands/removed/'"
            in error for error in errors))

    def test_launch_option_copy_and_grouping_are_not_engine_changes(self):
        def option(identifier, **fields):
            return {
                "id": identifier, "route_id": identifier.split(":")[1], "kind": "launch",
                "title": "Option", "description": "Before.", "audience": "developer",
                "availability": {"builds": ["release", "debug"]}, "syntax": "-OPT",
                "_provenance": {"source": "code/init.cpp", "guard": None}, **fields,
            }

        base = {"launch_options": [option("launch:copy"), option("launch:syntax")]}
        regrouped = {
            key: value for key, value in option("launch:copy").items() if key != "audience"}
        current = {"launch_options": [
            {**regrouped, "description": "After.", "group": "testing"},
            option("launch:syntax", syntax="-OPTION"),
        ]}
        delta = validate_manual.classify_command_deltas(current, base)
        self.assertEqual(delta["changed"], {"launch:syntax"})

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
            errors, {}, {}, {}, base_formats, changes, tombstones,
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


class DevelopmentAdditionTests(unittest.TestCase):
    def setUp(self):
        self.registry = {
            "development": "0.2.0",
            "by_version": {
                "0.1.0": {"status": "released"},
                "0.2.0": {"status": "development"},
            },
        }
        self.base_changes = {
            "new-entities": {
                "release": "0.2.0",
                "targets": [
                    {"type": kind, "id": "Sample", "effect": "added"}
                    for kind in ("command", "enum", "action", "event", "mission")
                ],
            },
        }
        self.changes = {
            identifier: {"data": deepcopy(data)}
            for identifier, data in self.base_changes.items()
        }

    def validate(self, entity_type="command", **overrides):
        scripting_delta = LifecycleEntityExpansionTests.empty_scripting_delta()
        arguments = {
            "changes": self.changes,
            "tombstones": [],
            "registry": self.registry,
            "base_changes": self.base_changes,
            "base_registry": self.registry,
        }
        if entity_type == "command":
            arguments["command_delta"] = {
                "added": set(), "changed": {"Sample"}, "removed": set(),
            }
        elif entity_type == "enum":
            arguments["enum_delta"] = {"changed": {"Sample"}}
        else:
            plural = next(
                plural for plural, (_, singular) in versioning.SCRIPTING_TYPES.items()
                if singular == entity_type)
            scripting_delta[plural]["shifted"] = {"Sample": (1, 2)}
        scripting_delta = overrides.pop("scripting_delta", scripting_delta)
        arguments.update(overrides)
        errors = []
        versioning.validate_branch_lifecycle(
            errors, {}, {}, {}, {},
            key_delta=LifecycleEntityExpansionTests.empty_key_delta(),
            scripting_delta=scripting_delta, **arguments)
        return errors

    def test_development_additions_cover_each_changed_entity_family(self):
        for kind in ("command", "enum", "action", "event", "mission"):
            with self.subTest(kind=kind):
                self.assertEqual(self.validate(kind), [])

    def test_command_category_change_remains_semantic_and_needs_no_new_record(self):
        command = LifecycleEntityExpansionTests.command("Sample", "sample", "Message")
        base = {"registered_commands": [{**command, "category": "Chat"}]}
        current = {"registered_commands": [{**command, "category": "Interface"}]}
        delta = validate_manual.classify_command_deltas(current, base)
        self.assertEqual(delta["changed"], {"Sample"})
        self.assertEqual(self.validate(command_delta=delta, base_commands=base), [])

    def test_amended_development_note_keeps_addition_coverage(self):
        self.changes["new-entities"]["data"]["title"] = "Describe the final behavior"
        self.assertEqual(self.validate(), [])

    def test_development_targets_can_move_between_records(self):
        self.changes["revised-additions"] = self.changes["new-entities"]
        self.changes["new-entities"] = {"data": {"release": "0.2.0", "targets": []}}
        self.assertEqual(self.validate(), [])

    def test_current_only_addition_does_not_cover_existing_behavior(self):
        self.assertEqual(self.validate(base_changes={}), [
            "command:Sample: add a change target with effect: changed",
        ])

    def test_released_additions_still_require_changed_targets(self):
        self.base_changes["new-entities"]["release"] = "0.1.0"
        self.changes["new-entities"]["data"]["release"] = "0.1.0"
        for kind in ("command", "enum", "action", "event", "mission"):
            with self.subTest(kind=kind):
                self.assertEqual(self.validate(kind), [
                    f"{kind}:Sample: add a change target with effect: changed",
                ])

    def test_released_versions_stop_covering_after_the_push_that_releases_them(self):
        registry = {
            "development": "0.3.0",
            "by_version": {
                "0.1.0": {"status": "released"},
                "0.2.0": {"status": "released"},
                "0.3.0": {"status": "development"},
            },
        }
        self.assertEqual(self.validate(registry=registry, base_registry=registry), [
            "command:Sample: add a change target with effect: changed",
        ])
        # The push that releases 0.2.0 straight from development still counts its records.
        self.assertEqual(self.validate(registry=registry), [])

    def test_missing_context_keeps_changed_target_requirements(self):
        for field in ("registry", "base_changes", "base_registry"):
            with self.subTest(field=field):
                self.assertEqual(self.validate(**{field: None}), [
                    "command:Sample: add a change target with effect: changed",
                ])

    def test_candidate_records_cover_only_when_the_range_merges_its_branch(self):
        registry = deepcopy(self.registry)
        registry["development"] = "0.3.0"
        registry["candidate"] = "0.2.0"
        registry["by_version"]["0.2.0"]["status"] = "candidate"
        registry["by_version"]["0.3.0"] = {"status": "development"}
        self.assertEqual(self.validate(registry=registry, base_registry=registry), [
            "command:Sample: add a change target with effect: changed",
        ])
        self.assertEqual(self.validate(
            registry=registry, base_registry=registry, merged_releases={"0.2.0"}), [])

    def test_merging_and_sealing_a_candidate_in_one_push_keeps_coverage(self):
        base_registry = deepcopy(self.registry)
        base_registry["development"] = "0.3.0"
        base_registry["by_version"]["0.2.0"]["status"] = "candidate"
        base_registry["by_version"]["0.3.0"] = {"status": "development"}
        registry = deepcopy(base_registry)
        registry["by_version"]["0.2.0"] = {"status": "released"}
        self.assertEqual(self.validate(
            registry=registry, base_registry=base_registry, merged_releases={"0.2.0"}), [])

    def test_current_addition_coverage_must_be_retained(self):
        for targets in ([], [{"type": "command", "id": "Sample", "effect": "deprecated"}]):
            with self.subTest(targets=targets):
                changes = {"new-entities": {"data": {"release": "0.2.0", "targets": targets}}}
                self.assertEqual(self.validate(changes=changes), [
                    "command:Sample: add a change target with effect: changed",
                ])

    def test_base_coverage_matches_exact_type_id_and_scope(self):
        for field, value in (("type", "enum"), ("id", "Other"), ("scope", "other")):
            with self.subTest(field=field):
                base_changes = deepcopy(self.base_changes)
                base_changes["new-entities"]["targets"][0][field] = value
                self.assertEqual(self.validate(base_changes=base_changes), [
                    "command:Sample: add a change target with effect: changed",
                ])

    def test_scoped_additions_do_not_cover_parent_or_sibling_scopes(self):
        target = {"type": "key", "id": "Scoped", "scope": "unit", "effect": "added"}
        base_changes = {"scoped": {"release": "0.2.0", "targets": [target]}}
        current = {"scoped": {"data": {
            "release": "0.2.0", "targets": [target, {**target, "scope": None}],
        }}}
        self.assertEqual(versioning.unreleased_additions(
            current, base_changes, self.registry, {"0.2.0"}), {("key", "Scoped", "unit")})
        current["scoped"]["data"]["targets"] = [{**target, "scope": "aircraft"}]
        self.assertEqual(versioning.unreleased_additions(
            current, base_changes, self.registry, {"0.2.0"}), set())

    def test_shifted_released_entity_still_needs_its_own_changed_target(self):
        delta = LifecycleEntityExpansionTests.empty_scripting_delta()
        delta["actions"]["shifted"] = {"Sample": (1, 2), "Released": (2, 3)}
        self.assertEqual(self.validate("action", scripting_delta=delta), [
            "action:Released: add a change target with effect: changed",
        ])

    def test_explicit_changed_target_still_works_without_addition_context(self):
        changes = {"change": {"data": {"release": "0.2.0", "targets": [
            {"type": "command", "id": "Sample", "effect": "changed"},
        ]}}}
        self.assertEqual(self.validate(changes=changes, base_changes=None), [])

    def test_development_addition_does_not_cover_a_removal(self):
        delta = {"added": set(), "changed": set(), "removed": {"Sample"}}
        tombstones = [{"type": "command", "id": "Sample", "route": "/commands/sample/"}]
        self.assertEqual(self.validate(command_delta=delta, tombstones=tombstones), [
            "command:Sample: add a change target with effect: removed",
        ])

    def test_new_entity_still_requires_an_addition_target(self):
        delta = {"added": {"Other"}, "changed": set(), "removed": set()}
        self.assertEqual(self.validate(command_delta=delta), [
            "command:Other: add a change target with effect: added",
        ])
        changes = deepcopy(self.changes)
        changes["new-entities"]["data"]["targets"].append(
            {"type": "command", "id": "Other", "effect": "added"})
        self.assertEqual(self.validate(command_delta=delta, changes=changes), [])

    def test_duplicate_addition_history_still_fails_validation(self):
        self.changes["duplicate"] = deepcopy(self.changes["new-entities"])
        errors = []
        versioning.validate_history(errors, self.registry, self.changes, [])
        self.assertIn("command:Sample: lifecycle may contain at most one added event", errors)


if __name__ == "__main__":
    unittest.main()
