---
title: Accept launch options carrying a long or quoted path
category: fix
release: 0.2.0
targets: []
credit: [ZivDero]
---

A launch option carrying a quoted path whose name contains spaces arrived as several arguments and was not recognized, because the command line was split on every space before any quoting was considered. The shell's own quoting rules now decide where one argument ends and the next begins.

An argument of about 125 characters or more crashed the game as it started, and a launch was capped at nineteen arguments; neither limit remains. A directory too long to have a file name appended to it is passed over by the file search rather than truncated into a different one.
