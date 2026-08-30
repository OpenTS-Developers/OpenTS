# Manual agent instructions

These instructions apply to `manual/` and supplement the repository-root
instructions.

## Read before editing

- Read [README.md](README.md) for the manual layout and launcher behavior.
- For public content, read [AUTHORING.md](AUTHORING.md) and
  [STYLE.md](STYLE.md).
- For schemas, generated data, adapters, lifecycle, routes, tooling, or
  publication, read [MAINTAINING.md](MAINTAINING.md).
- Before changing a factual claim, inspect the relevant generated record,
  authored page, current C++ source, and any loader or caller that determines
  the behavior.

## Working rules

- Use `python manual/tools/manage.py` as the contributor interface. Do not
  duplicate extraction or validation in an ad hoc script.
- Do not hand-edit `data/ini-keys.yaml`, `data/scripting.yaml`, or
  `data/commands.yaml`.
- Treat schemas, adapters, exclusions, aliases, tombstones, release data, and
  lifecycle records as compatibility contracts. Change them only when the task
  explicitly changes that contract.
- Give each fact one page owner and let structured fields render their own
  data. Do not copy generated lists into prose.
- Write change records under `changes/` as briefly as the change can be said,
  to the rules in [AUTHORING](AUTHORING.md). Detail belongs to the pages that
  own it.
- Support every public behavioral claim with the current source or a stated
  runtime observation. Narrow or omit claims that the evidence does not
  establish.
- Do not expose catalogs, extraction, or authoring provenance in public prose.
- Published routes are stable. Preserve an established URL with a redirect,
  alias, or tombstone when moving or removing a page. Never make a route change
  incidental to wording or filename cleanup.
- Keep game assets, original binaries, proprietary SDKs, credentials, personal
  data, and build output out of the repository.

## Workflow and handoff

Run `manage.py update` before and during engine-facing authoring. Use the
matching scaffold for a new page, replace every `TODO:`, and run
`manage.py check` before handoff. Use `serve` and inspect representative layouts
when presentation changes.

Inspect the final diff for generated churn, route changes, temporary files, and
changes outside the requested scope. Report exactly what ran and what did not.
Do not commit, push, deploy, or publish unless explicitly requested.
