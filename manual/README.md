# OpenTS manual source

`manual/` contains the public OpenTS manual for players, modders, and engine
contributors. Astro and Starlight build the site. Repository policy, build
instructions, and source conventions remain in
[CONTRIBUTING.md](../CONTRIBUTING.md), [BUILDING.md](../docs/BUILDING.md), and
[STYLE.md](../docs/STYLE.md).

The manual combines catalogs from the current engine with written guides.
Catalogs record what the source reads or registers; pages explain behavior,
structure, compatibility, and use.

## Governance

- [Authoring](AUTHORING.md) defines page ownership, evidence, lifecycle, and the
  contribution workflow.
- [Style](STYLE.md) defines the public manual's voice, terminology, examples,
  and Markdown vocabulary.
- [Maintaining](MAINTAINING.md) defines generated contracts, releases, routes,
  extraction changes, and publication behavior.
- [Agent instructions](AGENTS.md) apply these guides to agent work.

## Directory map

| Path | Purpose |
| --- | --- |
| `content/` | Authored pages and optional overlays |
| `changes/` | Authored engine lifecycle records |
| `data/` | Generated catalogs and maintained registries or adapters |
| `schema/` | JSON Schemas for generated data and authored frontmatter |
| `tools/` | Extraction, validation, scaffolding, and the contributor launcher |
| `site/` | The Astro/Starlight application, tests, and rendered-artifact checks |

## Setup

Python is pinned in `tools/.python-version`; Node and npm are pinned in
`site/.nvmrc` and `site/package.json`. Check the local toolchain from the
repository root:

```powershell
python manual/tools/manage.py doctor
```

Install missing pinned dependencies:

```powershell
python -m pip install -r manual/tools/requirements.txt
Set-Location manual/site
npm ci
Set-Location ../..
python manual/tools/manage.py doctor
```

`doctor --verbose` prints the resolved executables and version files.

## Contributor commands

Run every command from the repository root:

```powershell
python manual/tools/manage.py update
python manual/tools/manage.py serve
python manual/tools/manage.py check
python manual/tools/manage.py scaffold --help
```

The launcher commands are:

| Command | Behavior |
| --- | --- |
| `doctor` | Checks the pinned Python, Python packages, Node, npm, and installed site dependencies. It does not build the manual. |
| `update` | Regenerates `data/ini-keys.yaml`, `data/scripting.yaml`, and `data/commands.yaml` as one transaction, reports changes against a Git base, and runs structural and lifecycle validation. It changes tracked generated files when the engine catalog changes. |
| `serve` | Runs `update` and validation, then starts the Astro development server. |
| `check` | Regenerates into a temporary directory to detect drift, validates all contracts and lifecycle rules, runs the Python and site tests, and builds the site once with render, search, and link checks. It does not update the tracked catalogs. |
| `scaffold` | Creates minimal authored content for a supported page or change type and refuses to overwrite an existing file. Scaffolds remain invalid until every `TODO:` is replaced. |
| `release-notes` | Renders the change records assigned to one release as Markdown on standard output, for the release workflow and release authors. It changes nothing. |

`update`, `serve`, and `check` accept `--base-ref <revision>`. The default is
`HEAD`; continuous integration supplies the pull-request or pre-push revision.

`check` includes removed-entity fixtures, so its `dist` contains three pages
that a published build does not. Run `npm run build` in `site` for a publishable
artifact. CI also installs production-only site dependencies and verifies that
the clean build excludes the fixtures. `check` is the complete local gate, but
it does not reproduce every CI setup step.

Use `update` while changing engine-facing documentation, `serve` when visual
review is useful, and `check` before handoff. Continue with
[Authoring](AUTHORING.md) before editing public content.
