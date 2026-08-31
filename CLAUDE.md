# Claude Code instructions

`AGENTS.md` is the canonical instruction set for every agent. The files are
imported here so Claude receives their rules at the start of a session. Do not
repeat those rules in this file.

@AGENTS.md
@code/AGENTS.md
@manual/AGENTS.md

## Writing-rule hook

`.claude/settings.json` runs the shared `.agents/hooks/style-rules.py` after
every Edit and Write. Markdown edits re-inject the writing rules from
`AGENTS.md`. C and C++ edits under `code/` re-inject the comment rules from
`code/AGENTS.md` when they touch comments and report objective violations.
Codex uses the same script through `.codex/hooks.json`; the `AGENTS.md` files
remain the only source of the rules.
