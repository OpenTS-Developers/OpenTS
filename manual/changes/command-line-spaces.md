---
title: Accept launch options carrying a long or quoted path
category: fix
release: 0.2.0
targets: []
credit: [ZivDero]
---

A launch option carrying a path whose name contains spaces is now read as the
single argument it was written as. The command line was split on every space
before any quoting was considered, so a quoted path arrived as several
arguments and the option was not recognized. The shell's own quoting rules now
decide where one argument ends and the next begins.

An argument of about 125 characters or more no longer crashes the game as it
starts. Every argument is put through a transformation that recognizes the
options carrying no leading dash, and one long enough to fill that routine's
working buffer wrote its terminator one position past the end of it.

The number of arguments a launch can carry is no longer capped at nineteen.

A directory too long to have a file name appended to it is passed over by the
file search rather than truncated into a different one.
