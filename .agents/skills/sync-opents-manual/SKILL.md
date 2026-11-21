---
name: sync-opents-manual
description: 'Use when the current OpenTS branch or worktree contains engine changes and the manual under manual/ must be brought up to date. Derives the documentation scope from the branch diff instead of a fixed checklist, updates the owning pages, key scopes, lifecycle records, and catalog contracts, and verifies with the manual tooling.'
---

# Sync the OpenTS manual with the current branch

The branch defines the work. Establish what changed in this branch before reading or editing any page, and update only what those changes affect.

Paths in this skill are relative to the repository root.

## 1. Read the binding instructions

- `AGENTS.md`, `CONTRIBUTING.md`, and `docs/STYLE.md`.
- `manual/AGENTS.md`, `manual/README.md`, `manual/AUTHORING.md`, `manual/STYLE.md`, and `manual/MAINTAINING.md`.
- Read `docs/BUILDING.md` before making or repeating a build claim.

## 2. Establish the change set every run

Never assume the change set. Inspect it:

```powershell
git status --short
git branch -vv
git log --oneline --decorate -10
```

Pick the base the branch actually diverged from:

- the branch upstream from `git branch -vv` or `git rev-parse --abbrev-ref '@{u}'`;
- otherwise `upstream/main` or `origin/main`;
- otherwise the merge base the user names.

List and read the changes:

```powershell
git diff --stat <base>...HEAD
git diff <base>...HEAD -- code/
git diff --stat
git diff --cached --stat
git diff
```

Include uncommitted, staged, and untracked work. A branch may contain several related additions, fixes, and intermediate documentation commits; review all of them, and check whether an existing change record already covers part of the work. Read the manual pages and records the branch already touched as drafts to verify, not as settled text.

Classify each behavior change as preserved, fixed, or intentionally changed, and trace it in the source:

- Find the concrete reader, caller, initialization order, and first consequential use.
- When a shared helper replaces direct member access, follow **every** call site. The helper can add behavior the feature name does not mention, such as extra multipliers, fallbacks, or cached state. If the reach beyond the stated feature is unclear or contradicts existing documentation, ask the user whether to document it as intended or narrow the code first.
- Check boundaries: omission, zero, negative, fractional, rounding, clamping, and type variants where they differ.

## 3. Inventory the generated entities

```powershell
python manual/tools/manage.py update
```

Ask the user whether they would run it, or the agent should do it, If the user says it would run it, ask them to do so before starting. Then inspect the committed and uncommitted catalog deltas:

```powershell
git diff <base>...HEAD -- manual/data
git diff -- manual/data
```

New or changed keys, key scopes, commands, trigger actions, events, and missions need authored pages and lifecycle targets. Never edit `manual/data/ini-keys.yaml`, `manual/data/scripting.yaml`, or `manual/data/commands.yaml` by hand. `commands.yaml` and `scripting.yaml` may appear modified only by line endings; confirm with `git diff` before reporting.

A key that now shares its spelling between different readers gains multiple scopes. The catalog marks those scopes `needs_review` until `manual/data/adjudications.yaml` records the human verdict. Add one entry per colliding identity (`DeclaringClass::member`) when the meanings are genuinely distinct, then run `update` again.

## 4. Choose the page owner

Use the table in `manual/AUTHORING.md` under "Choose the fact owner". In practice:

- A new or changed INI assignment owns a key page: parsing, accepted values, omission, and one statement of its effect.
- A formula, ordering, limit, or interaction owns the system page. Do not copy the calculation into a key page or the values into a system page.
- A new script or command entity owns its generated-page route.
- Visible engine additions and behavior changes own a record under `manual/changes/`.
- Architecture and ownership notes belong under `manual/content/internals/`.

Every new page starts from `python manual/tools/manage.py scaffold`. Search the target folder first; scaffold refuses to overwrite an existing page. Replace every `TODO:`, then render or validate.

## 5. Key pages, scopes, and omission

- A single-scope key uses one page, `manual/content/keys/<route>.md`, with `when_omitted` on that page. A shared scope still counts as one scope; do not add a `--<scope>` page.
- A multi-scope key keeps an unscoped hub page with `key` and `summary` and no `when_omitted`, plus one `<route>--<scope>.md` page per scope with `scope:`, `label:`, and its own `when_omitted`. A hub body is optional and renders after the scope sections.
- The hub summary renders in reference tables for every scope, so it must describe the key as a whole. A scope label becomes that scope's section heading and its row label in multi-reading tables.
- `when_omitted` on an unscoped page of a multi-scope key is invalid. Validation requires omission metadata for every scope of an added or behavior-changed key, and names the missing scope.
- The scope route id is the slug of the first `applies_to` entry, for example the first object type a shared techno read applies to. Scope anchors are `/keys/<route>/#scope-<scope>`.
- When an existing key gains a second meaning, split its page and rewrite the hub summary. Keep the established `/keys/<route>/` URL as the hub.
- A new key added to the catalog is an addition target; a new scope of an existing key is an addition target that names that scope.

## 6. Update every page that states the old behavior

Link facts to their owner; do not copy rendered fields such as file, section, applicability, accepted values, omission, or command metadata into prose.

Search public content for the old claim, not just the owning page:

```powershell
rg -n "old term|old key|old limit" manual/content
```

Use the grep tool when `rg` is not installed. Follow links in both directions. A key page, a system page, and a change record may all need to change for one behavior. Remove or narrow claims that the new source contradicts, including claims about cached state, defaults, or type variants in adjacent pages.

## 7. Lifecycle records

Check `manual/data/releases.yaml` and the existing records before creating one.

- Do not add a record for documentation-only work, source-location fixes, extraction corrections, or catalog migrations.
- For an adjustment to an addition in the current development release, amend the existing addition record instead of adding another. Keep its target set and release.
- A new engine entity or a change to released behavior needs a record in the active development release.
- Target every entity the reader reaches: added keys with their scope, a changed system whose behavior changed, and any other changed owner. Use `breaking: true` and ordered `migration` steps only for a real compatibility break.
- A removed engine entity needs a tombstone at its established route; moving a page needs a redirect or alias. Never change a route as part of prose work.
- Keep the body to the visible difference: two or three sentences per visible change, no development narrative, no internal fields, and no restatement.

## 8. Verify before handoff

```powershell
python manual/tools/manage.py check
```

Ask the user whether they will run `check` themselves or the agent should. If the user will run it, ask them to run it and report the result before the handoff; do not run it yourself.

- Run `update` before `check` when the tracked catalog must absorb regeneration or adjudication changes; `check` regenerates and compares, so a stale tracked file fails as drift.
- Do not run `update` or `check` if the user says they will; say so in the handoff instead.
- For presentation changes, use `serve` and inspect representative desktop and narrow layouts.
- Inspect the final diff for generated churn, route changes, temporary files, and out-of-scope edits. Keep unrelated dirty-worktree changes intact.

## 9. Handoff

Report:

- the branch, base, and change set reviewed;
- each behavior and the manual pages that now own it;
- the generated-catalog delta, or that it is empty;
- the exact commands run and their results;
- the checks not run and why.

Do not commit, push, or open a pull request unless the user explicitly asks.

## Traps

- A confined shell may be denied reads of freshly written generated files while the file-reading tools still work. Use the file tools and report the access problem only if it blocks the work.
- Do not infer the base from a branch name. Confirm the upstream and read the diff.
- A state change or helper substitution can affect callers that never mention the feature. Trace them.
- A key that gains a scope changes the omission and scaffold contract even though its spelling did not change.
- An existing route that now needs a split must remain reachable. Never change a route to fix prose.
