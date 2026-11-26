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

A launch option's adapter also names its `group`. An option belongs to
`debug-build` and lists only the Debug build exactly when every one of its
parser sites is compiled under `_DEBUG`; the extractor refuses any other
combination. An adapter's title, description, audience and group are authored
copy, so changing them is not an engine change.

The branch check accepts an existing unreleased addition record for later changes to a command, enum signature, or scripting index. The addition target must match the entity and scope in both the Git base and the current tree. Its version must be unreleased in the base, and must be the development version or a candidate whose release branch the checked range merges. A current-only addition target does not satisfy a required `changed` target. Follow the [authoring rules](AUTHORING.md#lifecycle-records) when updating addition prose.

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

### Release registry

`data/releases.yaml` lists every version in complete SemVer 2.0 without build
metadata. Each entry has one status:

| Status | Meaning | Limit |
|---|---|---|
| `development` | The version this branch builds | Exactly one, the highest entry |
| `candidate` | A version being stabilized on its release branch; only main lists it | At most one, higher than every released version |
| `released` | A published version, with its ISO `date` | Status and date never change |

The development version's numeric core must match CMake's
`project(OpenTS VERSION ...)` declaration, and its prerelease label must match
CMake's `OPENTS_VERSION_PRERELEASE`, which is empty when there is no label. The
`Engine release` workflow publishes only versions without a registry label: a
release candidate's label comes from its tag. The private npm package version is
tooling metadata, not the OpenTS release.

A version moves from `development` to `candidate` or `released`, and from
`candidate` to `released`. Nothing returns to `development`. A candidate row may
be added, as when a patch version is first merged into main, and may be removed
while no record targets it.

### Save and network compatibility

The engine stamps saves and network sessions with the version's numeric core,
`major.minor.patch`. Every build of one version shares that stamp: development
snapshots, release candidates, release branch builds and the final release.
They accept each other's saves and games with no promise that they work
together. Different cores never match, so 0.2.1 refuses 0.2.0 saves and games,
and 0.3.0 development snapshots refuse both. See
[Compatibility boundaries](../CONTRIBUTING.md#compatibility-boundaries).

### Release branches

A minor version is stabilized on `release/<major>.<minor>`, cut from main. The
branch's registry keeps the version as `development`. Main lists it as the
`candidate` and opens the next minor version as `development`.

- Fixes for the candidate land on the release branch, directly or through a
  pull request into it, and reach main by merging the branch with a merge
  commit. Never squash or rebase that merge.
- The release branch owns the candidate's change records. On main they change
  only through a merge of the branch.
- New features wait for the next version on main.
- Never merge main, or a branch based on main, into a release branch. The
  release branch would take main's version, the check would not catch it, and
  the next tag's preflight would refuse the release.
- Only maintainers merge a release branch, and only into main. A pull request
  that carries such a merge passes its own check but fails on main once
  squash-merged.

### Release checks

`manage.py check` enforces the rules below. It reads the release tags and every
local and remote copy of each `release/*` branch, so fetch them from the OpenTS
repository first with `git fetch --tags <remote>`, where `<remote>` is `origin`
in a clone of OpenTS and usually `upstream` in a fork. A missing tag or branch
fails the check. Rules marked
"base" compare with the check's base revision and treat a version as released
only if the base already released it, so one push can merge the release branch
and seal its version.

| Rule | Fails when |
|---|---|
| New record (base) | a new change record targets a version that is neither the development version nor the candidate |
| Moved record (base) | a record's `release` changes, unless the old and new versions are both development or candidate |
| Released record (base) | a released record's category, targets or breaking state changes |
| Stable ID (base) | a change record present in the base is removed or renamed |
| Candidate mirror | the registry lists a candidate, and its records or their text differ from those at the newest commit of `release/<major>.<minor>` that HEAD or a merge in progress contains |
| Released tag | tag `v<version>` of a released version is missing or not merged into HEAD, or its records differ from HEAD's in IDs or lifecycle fields; prose may differ |
| Closed branch | HEAD descends from, and is not, the tag of its own development version |
| Coverage (base) | a detected engine change has no matching target in a record of the current or base development version, or in a candidate record when the checked range merges the candidate's release branch |
| RC note (base) | a new RC note targets a version that is neither the development version nor the candidate |

A merge of the release branch lets candidate records cover every engine change
in the push, so keep a merge push to the merge.

### Release notes

`python manual/tools/manage.py release-notes <version>` writes one release's
change records as Markdown to standard output. Breaking records come first with
their migration steps, then every record grouped by category, so a breaking
record is listed twice. The command refuses a version that no record targets.

With `--tag <tag>`, the records come from that tag, and the notes add the
changes since the previous candidate. The previous candidate is the highest
`v<version>-<label>` tag that the tag contains and that sorts below it. Labels
compare digit runs as numbers, so `rc10` follows `rc9`, and a final sorts above
all its candidates.

| Tag | Generated notes |
|---|---|
| First candidate, or a final with no candidates | Every record |
| Later candidate | Records added or edited since the previous candidate and RC notes added since it, then every record in a collapsed block |
| Final after candidates | Every record, then the records added or edited since the last candidate |

A later candidate or a final also links to the commits since the previous
candidate when `GITHUB_SERVER_URL` and `GITHUB_REPOSITORY` are set, as they are
in GitHub Actions.

### Publishing workflow

Publishing a GitHub release runs the `Engine release` workflow. Its preflight
job stops the run before anything is built when:

- the tag no longer points at the commit the run started for;
- the tag is not `v<major>.<minor>.<patch>`, optionally followed by
  `-<label>`;
- the tag's numeric core differs from the CMake version at the tag;
- the release's pre-release mark disagrees with the tag: a tag with a label
  needs the mark, and a final must not have it;
- a tag of a later minor version exists;
- the tagged commit is not on `origin/release/<major>.<minor>`;
- `release-notes --tag` fails.

The build passes the tag's label to CMake and fails unless the built version is
exactly `<core>-<label>`, or `<core>` for a final. The workflow then attaches one
zip per platform and writes the generated notes into the release body between
two marker comments. Write your own text above the markers; a rerun replaces
only the text between them.

If the release itself was wrong, such as its pre-release mark, correct it on
GitHub and rerun the workflow from the Actions tab. Editing the release does not
rerun it. If the tag was wrong, delete the release and the tag, tag the right
commit, and publish a new release. Do not move a tag and rerun: a rerun builds
the commit the tag named when the run started, and the preflight refuses it.

### Cut a release and publish the first candidate

1. Choose the main commit that contains everything the release ships, and
   confirm that main's registry lists no candidate.
2. Create `release/<major>.<minor>` at that commit and tag `v<version>-rc1` on
   it. Push both.
3. Publish a GitHub release from the tag with "Set as a pre-release" ticked.
   Tell testers that every build of the version shares one save stamp, so they
   should all use the same build.
4. On main, set the version to `candidate` in `data/releases.yaml`, add the next
   minor version as `development`, and set CMake to it. A record added or
   amended on main after the cut commit fails the check here; move it to the
   next version or restore the cut's copy. Run
   `python manual/tools/manage.py check`, commit, and push.

To release without candidates, tag the cut `v<version>`, publish a full release,
and in step 4 set the version to `released` with today's date. Push step 4
right after the tag: until then, the check fails on every main commit after the
cut.

### During the candidate period

- **A bug in the candidate:** fix it on the release branch, and add or amend its
  record there as [Lifecycle records](AUTHORING.md#lifecycle-records)
  describes. Then merge the branch into main:
  1. Run `git fetch origin` and
     `git merge --no-ff --no-commit origin/release/<major>.<minor>`, so that
     fixes merged through pull requests are included and the merge waits for
     the regenerated catalogs. Resolve source conflicts so that both behaviors
     survive. Add a development record only if main's result differs visibly
     from the candidate's.
  2. Run `python manual/tools/manage.py update` to regenerate
     `data/ini-keys.yaml`, `data/scripting.yaml` and `data/commands.yaml`. Do
     not resolve those files by picking one side.
  3. Run `python manual/tools/manage.py check`, commit the merge, and push.
- **A fix that landed on main first:** cherry-pick it onto the release branch
  and set its record's `release` to the branch's version there. When the next
  merge into main conflicts on that record, take the branch's copy of the
  record.

### Later candidates and the final

Tag the release branch tip `v<version>-rc<n>` and publish a pre-release. For the
final:

1. Tag the release branch tip `v<version>` and publish a full release. Tag the
   tip: a record committed to the branch after the tag falls outside the
   release, and the seal in step 2 fails.
2. On main, merge the release branch so that the tag is merged; Git reports
   "Already up to date" when main already contains it. Set the version to
   `released` with today's date. Both may go in one push.

After the final, the check fails on every new commit on the release branch
until a patch version is opened there.

### Patch releases

1. On the release branch, set the released version to `released` with the date
   main gave it, add the next patch version as `development`, and set CMake to
   it. Commit this before any fix.
2. Fix on the branch. Records target the patch version, and a fix to behavior
   the released version shipped needs a record of its own.
3. Merge into main. The first merge conflicts on the version lines: keep main's
   CMake version and development version, and list the patch version as
   `candidate`.
4. Tag `v<patch version>` on the branch tip and publish it, then merge into
   main and seal it as for a final.

A patch release has no candidates unless you tag one; RC notes then work as
for a minor release. Patches for a minor version end when the next minor
version is cut, because main lists one candidate at most and the preflight
refuses a tag once a later minor tag exists. Release an open patch version
before cutting, or remove its candidate row while no record targets it.

### Baseline and removals

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

Command line options have no pages of their own. Each option's established
route, `/using/command-line/<route>/`, redirects to its entry at
`/using/command-line/#<route>`. When an option is removed, its tombstone takes
over that route.

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

`DOCS_REVISION` renders on every page and is excluded from the search index.
Pagefind splits a hash into letter and digit runs, so indexing it would put
tokens such as `6` on every page and flatten the ranking that the artifact
search check asserts. Keep any other per-build value out of the index for the
same reason.

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
