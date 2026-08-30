# Claude Code instructions

`AGENTS.md` is the canonical instruction set for every agent. The files are
imported here so their rules are in context from the start of a session rather
than behind a link. Do not restate their rules in this file.

@AGENTS.md
@code/AGENTS.md
@manual/AGENTS.md

## Comment-rule hook

`.claude/settings.json` runs `.claude/hooks/comment-rules.py` after every Edit
and Write. For C and C++ files under `code/`, an edit that touches comment
lines gets the comment sections of `code/AGENTS.md` re-injected as context,
and added comments that break those rules come back as findings to fix
immediately. `code/AGENTS.md` stays the single owner of the rules.
