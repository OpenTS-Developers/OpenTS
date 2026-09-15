---
title: Strip quotes from launch options
category: fix
release: 0.1.0
targets: []
credit: [ZivDero, tomsons26]
---

A launch option is now matched as though its double quotes were not there, whether they wrap the whole option or part of it. Arguments used to be compared exactly as the shell handed them over, so a quoted argument went unrecognized, and a path containing a space obliges the quotes.
