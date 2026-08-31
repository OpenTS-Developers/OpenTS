# Maintaining the manual system

This guide covers extraction, validation, releases, routes, and publication.
For ordinary content work, start with [Authoring](AUTHORING.md).

## Authorities

| Concern | Authority |
| --- | --- |
| Python version and packages | `tools/.python-version` and `tools/requirements.txt` |
| Node and npm versions | `site/.nvmrc` and `site/package.json` |
| Generated INI, scripting, and command records | Current engine source plus the extractors invoked by `tools/manage.py` |
| Data and frontmatter shapes | JSON Schemas under `schema/` |
| Releases | `data/releases.yaml` |
| Engine lifecycle | Markdown records directly under `changes/` |
| Removed public entities | `data/tombstones.yaml` |
| Numeric scripting compatibility paths | `data/scripting-route-aliases.yaml` |
| Direct controls and launch parser classification | `data/command-adapters.yaml` |
| Exceptional INI read classification | `data/adjudications.yaml` and `data/ini-read-exclusions.yaml` |
| Site dependency graph | `site/package-lock.json` |

Python and Astro consume the same schemas through adapters. Changing a schema
changes the manual format, so update both consumers and their tests. Do not
change a schema, extractor, adapter, exclusion, route registry, or tombstone
just to silence validation.

## Generation and validation

Use `tools/manage.py` as the contributor interface. `update` runs `extract.py`,
`scripting.py`, and `commands.py`, checks that all three produced output, then
replaces the tracked catalogs. It rolls back on an operating-system failure;
launcher changes must keep the transaction atomic.

`check` writes the same three files to a temporary directory and compares them
with the tracked copies before validating authored data and checking the site.
It must not change the tracked catalogs.

The repository-wide typed-INI inventory is fail-closed. Exclude only a genuinely
non-public or exceptional read, with a durable reason. Where an enrolled reader
owns the site, an `excluded` rule also removes its read from the catalog so
assembly and classification agree. Fix public scope disagreements in the
extractor or an explicit adjudication.

An entry name is everything left of `=`, including digits. In a read, it is
always the argument after the section. The extractor and inventory scanners
must share this contract; otherwise a site can be extracted but never
inventoried. `tools/extraction_history.py` also uses the contract, so changing
either scanner affects how catalog deltas are classified.

Generated provenance records stable identities, not source positions. A scope
cites the read's file and the class and member it fills; a command cites its
registering class. Line numbers are extraction diagnostics and are never
serialized, so moving code alone cannot drift a tracked catalog.
`tools/extraction_history.py` relocates a key's reads by scanning the cited file
in both revisions with the shared inventory scanner and comparing accessor
statement text. This distinguishes an extractor coverage correction from an
engine change without relying on a read's position.

A key's recorded content, not extraction order, determines its scopes. Two
readers making the same read for one concrete type fold into the declaration
covering the widest family, so adding a unit cannot split or merge a published
scope. `tools/tests/test_scope_coalescing.py` reverses the unit lists during
re-extraction and checks every key against its published scopes.

A reading is identified by its scope route. Only `allKeys` resolves these
routes because it distinguishes readings that would otherwise share one. Every
table listing a section resolves its rows through `allKeys`. A row whose route
came from elsewhere takes the first reading's omission and effect instead of
its own. If a section reads one spelling more than once, the authored `label:`
distinguishes the rows. Leave out a reading that does nothing, but keep the
spelling live where it works.

Command discovery is also fail-closed. Objects registered through
`AllCommands` form the rebindable command catalog. Every discovered direct key
handler and launch-parser branch needs exactly one public adapter or one
reasoned exclusion. Command IDs are case-sensitive. Do not infer default
bindings from a declaration or nearby code.

Enums are authored selections backed by explicit source adapters. Documenting
an existing fixed domain is documentation work, not an engine change. Its
adapter must preserve constants, stored values, public tokens, and order.
Dynamic registries are not enum adapters.

Formats are authored contracts. Their structured fields own filenames,
extensions, registrations, positional fields, companions, and key-scope
selectors. The four AI scripting formats keep these compatibility
routes:

- `/mapping/team-types/`
- `/mapping/task-forces/`
- `/mapping/scripts/`
- `/mapping/ai-triggers/`

Other format routes default to `/formats/<filename-stem>/`.

## Releases and lifecycle

`data/releases.yaml` uses complete SemVer 2.0 versions without build metadata.
It has exactly one `development` version, which must be the highest entry; only
`released` entries carry an ISO date. The development version's numeric core
must match CMake's `project(OpenTS VERSION ...)` declaration. Its prerelease
label must match CMake's `OPENTS_VERSION_PRERELEASE`, which is empty when there
is no label. The private npm package version is tooling metadata, not the OpenTS
release.

The engine stamps saves and network sessions with this version. Opening a cycle
retires the previous cycle's saves. Snapshots from one active cycle share a
stamp but have no interoperability promise, and several compatibility breaks
may accumulate before release. See
[Compatibility boundaries](../CONTRIBUTING.md).

To publish a release:

1. Confirm that the development entry names the release and that the commit to
   be tagged contains everything it ships.
2. Create and publish the GitHub release from a `v<version>` tag on that
   commit. The `Engine release` workflow builds the tag, attaches the packaged
   zip, and appends
   `python manual/tools/manage.py release-notes <version>` output to the release
   body.
3. Tag before opening the next development cycle. The tagged commit's CMake
   version must still name the release.

`release-notes` writes one release's change records as Markdown to standard
output. Breaking changes and their migration steps come first, followed by the
remaining records grouped by category. It reads the records validated by the
lifecycle checks and refuses a version that no record targets.

To open the next development cycle:

1. Mark the current development entry `released` and add its ISO release date.
2. Add one higher development version.
3. Update the CMake project version if the numeric core changed and the CMake
   prerelease label if the label changed.
4. Run `python manual/tools/manage.py check`.

Do not move existing change records into the new cycle; release assignments are
stable. Once released, a record's category, targets, breaking state, and
migration steps are immutable. Released registry entries and dates are also
immutable.

The catalog from the start of structured lifecycle tracking is the baseline;
its entities have no addition event. New engine entities and deliberate
behavior changes need records in the current development release. A removed
entity needs one authoritative removal target and a tombstone at its established
route. Tombstones stay out of active indexes but remain available through direct
navigation and search.

## Public routes

Artifact checks derive expectations from the content tree, `data/`, and
`site/src/i18n/en.mjs`, then compare them with the built HTML. Write an
expectation by hand only for a judgement: retired copy that must not return,
badge assignments, top-level view order, or a route-stability contract. Adding
a page must not require changing a check.

Published routes are stable. Moving or removing a page requires a redirect,
alias, or tombstone at its established URL and an artifact-level test. Changing
a title must not change its route by accident.

Numeric scripting indices are serialized engine identities. Their compatibility
paths live in `data/scripting-route-aliases.yaml`; never assign a reserved
numeric path to another engine ID. Removed keys, scripting entities, formats,
enums, systems, and commands use tombstones. A tombstone takes its removal
version from the lifecycle record rather than duplicating it.

Every route change must be deliberate and reviewed with the rendered route
diff.

## Publication

The site reads these build-time settings:

| Variable | Meaning |
| --- | --- |
| `DOCS_SITE` | Deployment origin without a path |
| `DOCS_BASE` | Repository or preview path prefix |
| `DOCS_REPOSITORY_URL` | Source repository URL used for source and feedback links |
| `DOCS_REVISION` | Revision used in source links and feedback metadata |
| `DOCS_DEMO` | Explicitly marks an alternate build as a demo |

The Pages workflow derives the repository URL and project path from GitHub's
repository context, so it works in staging and the final OpenTS repository.
Builds under `/Docs-Demo` or with `DOCS_DEMO=1` omit the community link reserved
for the official publication.

CI runs the full manual check, installs production-only dependencies, rebuilds,
and verifies the artifact before upload. The check includes synthetic
removed-entity fixtures, the manual's only tombstone pages, so it checks a
superset of the published pages. Only the workflow rebuild runs without those
fixtures and proves they are absent from a publishable artifact. A separate
test, which needs no build, covers the variable that admits them. Changes must
keep these two checks paired.

## Maintainer validation

The [Public routes](#public-routes) rule for artifact checks also applies to the
Python and Node contract suites. Derive expectations that mirror generated
data. Write out only judgements such as a frozen route, stored enum value, or
forced key binding. A record count is never a judgement; assert what every
record satisfies instead.

Run the narrowest affected tests first. Before handoff, run:

```powershell
python manual/tools/manage.py check
```

For a visible site change, use `serve` to inspect representative desktop and
narrow layouts. For a governance change, run the repository Markdown link
checker and confirm that rendered source, content, generated data, and route
inventory did not change. Report exact results and any relevant checks not run.
