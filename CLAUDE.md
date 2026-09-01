# Claude Code instructions

`AGENTS.md` is the canonical instruction set for every agent. The files are
imported here so Claude receives their rules at the start of a session. Do not
repeat those rules in this file.

@AGENTS.md
@code/AGENTS.md
@manual/AGENTS.md

## Writing-rule hook

`.claude/settings.json` runs the shared `.agents/hooks/style-rules.py` on
five events. Edits and writes are checked from their payloads; shell and
script writes are caught by a git scan against a session baseline recorded
at SessionStart, so no edit path bypasses the checks. Markdown edits
re-inject the writing rules from `AGENTS.md`; C and C++ edits under `code/`
re-inject the comment rules from `code/AGENTS.md` when they touch comments
and report objective violations. Subagents receive both rule sections at start, and compaction
re-injects them. Stop blocks while objective violations remain, and blocks
once per session to require a review of changed prose and comments against
the rules. Codex uses the same script through `.codex/hooks.json` with the
per-edit subset; the `AGENTS.md` files remain the only source of the rules.
