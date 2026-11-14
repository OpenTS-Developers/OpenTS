"""Dependency-light contributor commands and safe scaffold facade."""

import sys

import contributor_catalogs
import contributor_engine as _engine
from contributor_engine import *  # noqa: F401,F403 - preserve helper API


# Keep these facade values patchable for focused tests and embedders.
ROOT = _engine.ROOT
MANUAL = _engine.MANUAL
TOOLS = _engine.TOOLS
DATA = _engine.DATA
SITE = _engine.SITE
_version_output = _engine._version_output


LIFECYCLE_CATEGORIES = ("feature", "fix", "balance", "performance", "internal")
LIFECYCLE_TARGETS = (
    "key", "action", "event", "mission", "format", "enum", "system", "command"
)


def _sync():
    _engine.ROOT = ROOT
    _engine.MANUAL = MANUAL
    _engine.TOOLS = TOOLS
    _engine.DATA = DATA
    _engine.SITE = SITE
    _engine._version_output = _version_output


def doctor(verbose=False):
    _sync()
    return _engine.doctor(verbose)


def command_doctor(arguments):
    verified, actions = doctor(arguments.verbose)
    print("VERIFIED")
    for message in verified:
        print(f"  OK  {message}")
    if actions:
        print("ACTION REQUIRED")
        for message in actions:
            print(f"  - {message}")
        return 1
    print("ACTION REQUIRED\n  none")
    return 0


def scaffold_key(identifier, scope=None):
    _sync()
    return _engine.scaffold_key(identifier, scope)


def scaffold_scripting(identifier):
    _sync()
    return _engine.scaffold_scripting(identifier)


def scaffold_enum(identifier):
    _sync()
    return _engine.scaffold_enum(identifier)


def scaffold_system(identifier):
    _sync()
    return contributor_catalogs.scaffold_system(_engine, identifier)


def scaffold_guide(identifier):
    _sync()
    return contributor_catalogs.scaffold_guide(_engine, identifier)


def scaffold_format(identifier, kind):
    _sync()
    return contributor_catalogs.scaffold_format(_engine, identifier, kind)


def scaffold_command(identifier):
    _sync()
    return contributor_catalogs.scaffold_command(_engine, identifier)


def scaffold_using(identifier, category):
    _sync()
    return contributor_catalogs.scaffold_using(_engine, identifier, category)


def release_notes(version):
    _sync()
    return _engine.release_notes(version)


def command_release_notes(arguments):
    try:
        sys.stdout.write(release_notes(arguments.version))
    except (OSError, ValueError) as error:
        print(f"ACTION REQUIRED\n  - {error}", file=sys.stderr)
        return 1
    return 0


def scaffold_change(arguments):
    _sync()
    if arguments.scope and arguments.target_type != "key":
        raise ValueError("--scope is allowed only for key lifecycle targets")
    breaking = bool(getattr(arguments, "breaking", False))
    migration = list(getattr(arguments, "migration", []) or [])
    path = MANUAL / "changes" / f"{_engine._slugify(arguments.identifier)}.md"
    frontmatter = {
        "title": arguments.title or _engine._display_name(arguments.identifier),
        "category": arguments.category or "feature",
        "release": _engine._development_release(),
        "targets": [{
            "type": arguments.target_type,
            "id": arguments.target_id,
            **({"scope": arguments.scope} if arguments.scope else {}),
            "effect": arguments.effect,
        }],
    }
    if breaking:
        frontmatter["breaking"] = True
        frontmatter["migration"] = migration or [
            "TODO: replace with a concrete migration step"
        ]
    frontmatter["credit"] = arguments.credit or ["TODO: name the author"]
    _engine._write(
        path,
        frontmatter,
        "TODO: explain the player- or modder-visible change and compatibility impact.",
    )


def command_scaffold(arguments):
    try:
        if arguments.entity_type == "key":
            scaffold_key(arguments.identifier, arguments.scope)
        elif arguments.entity_type == "scripting":
            scaffold_scripting(arguments.identifier)
        elif arguments.entity_type == "enum":
            scaffold_enum(arguments.identifier)
        elif arguments.entity_type == "system":
            scaffold_system(arguments.identifier)
        elif arguments.entity_type == "guide":
            scaffold_guide(arguments.identifier)
        elif arguments.entity_type == "format":
            scaffold_format(arguments.identifier, arguments.kind)
        elif arguments.entity_type == "command":
            scaffold_command(arguments.identifier)
        elif arguments.entity_type == "using":
            scaffold_using(arguments.identifier, arguments.category)
        else:
            scaffold_change(arguments)
    except (OSError, ValueError) as error:
        print(f"ACTION REQUIRED\n  - {error}")
        return 1
    return 0


def add_parsers(commands):
    doctor_parser = commands.add_parser(
        "doctor", help="diagnose the local documentation toolchain")
    doctor_parser.add_argument(
        "--verbose", action="store_true",
        help="show resolved executable and version-authority paths")

    scaffold = commands.add_parser(
        "scaffold", help="create safe minimal authored content")
    scaffold.add_argument("entity_type", choices=(
        "key", "scripting", "enum", "system", "guide", "format", "command",
        "using", "change",
    ))
    scaffold.add_argument("identifier")
    scaffold.add_argument("--scope")
    scaffold.add_argument(
        "--kind", choices=("syntax", "file", "registry", "record", "binary"))
    scaffold.add_argument("--target-type", choices=LIFECYCLE_TARGETS)
    scaffold.add_argument("--target-id")
    scaffold.add_argument(
        "--effect", choices=("added", "changed", "deprecated", "removed"),
        default="added")
    scaffold.add_argument("--category", choices=(
        *LIFECYCLE_CATEGORIES,
        *contributor_catalogs.USING_CATEGORIES,
    ))
    scaffold.add_argument("--title")
    scaffold.add_argument("--credit", action="append", default=[])
    scaffold.add_argument("--breaking", action="store_true")
    scaffold.add_argument("--migration", action="append", default=[])

    notes = commands.add_parser(
        "release-notes",
        help="render one release's change records as Markdown on stdout")
    notes.add_argument("version", help="release version, such as 0.1.0")


def validate_scaffold_arguments(parser, arguments):
    if arguments.command != "scaffold":
        return
    entity_type = arguments.entity_type
    if entity_type == "change":
        if not arguments.target_type or not arguments.target_id:
            parser.error("scaffold change requires --target-type and --target-id")
        if arguments.category and arguments.category not in LIFECYCLE_CATEGORIES:
            parser.error("scaffold change --category must be a lifecycle category")
        if arguments.kind:
            parser.error("--kind is valid only for scaffold format")
        if arguments.migration and not arguments.breaking:
            parser.error("--migration requires --breaking")
        return

    if entity_type == "using":
        if arguments.category not in contributor_catalogs.USING_CATEGORIES:
            parser.error("scaffold using requires a Using OpenTS --category")
    elif arguments.category:
        parser.error("--category is valid only for scaffold using or scaffold change")

    if entity_type == "format":
        if not arguments.kind:
            parser.error("scaffold format requires --kind")
    elif arguments.kind:
        parser.error("--kind is valid only for scaffold format")

    if arguments.scope and entity_type != "key":
        parser.error("--scope is valid only for scaffold key or scaffold change")
    if any((
            arguments.target_type,
            arguments.target_id,
            arguments.effect != "added",
            arguments.title,
            arguments.credit,
            arguments.breaking,
            arguments.migration,
    )):
        parser.error("lifecycle target options are valid only for scaffold change")
