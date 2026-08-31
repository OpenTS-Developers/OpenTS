# Manual agent instructions

These instructions apply to `manual/` and supplement the repository-root
instructions.

## Read before editing

- Read [README.md](README.md) for the manual layout and launcher behavior.
- Read [AUTHORING.md](AUTHORING.md) and [STYLE.md](STYLE.md) before changing
  public content.
- Read [MAINTAINING.md](MAINTAINING.md) before changing schemas, generated
  data, adapters, lifecycle, routes, tooling, or publication.
- Check factual claims against the relevant record, page, source, loader, and
  caller.

## Working rules

- Use `python manual/tools/manage.py` as the contributor interface. Do not
  duplicate extraction or validation in an ad hoc script.
- Do not hand-edit `data/ini-keys.yaml`, `data/scripting.yaml`, or
  `data/commands.yaml`.
- Schemas, adapters, exclusions, aliases, tombstones, release data, and
  lifecycle records are compatibility contracts. Change them only when the
  task explicitly changes the contract.
- Give each fact one page owner and let structured fields render their own
  data. Do not copy generated lists into prose.
- Keep `changes/` records brief. Detail belongs on the page that owns it.
- Support every public behavioral claim with the current source or a stated
  runtime observation. Narrow or omit claims that the evidence does not
  establish.
- Do not expose catalogs, extraction, or authoring provenance in public prose.
- Published routes are stable. Preserve an established URL with a redirect,
  alias, or tombstone when moving or removing a page. Never make a route change
  incidental to wording or filename cleanup.

## Workflow and handoff

Run `manage.py update` before and during engine-facing authoring. Scaffold new
pages, replace every `TODO:`, and run `manage.py check` before handoff. Use
`serve` to inspect representative layouts after presentation changes.

Inspect the final diff for generated churn, route changes, temporary files, and
out-of-scope edits. Report exactly what ran and what did not.
