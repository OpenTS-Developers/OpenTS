---
title: Start without asking Windows whether a mouse is attached
category: fix
release: 0.1.0
targets: []
credit: [ZivDero]
---

Startup no longer refuses to run when Windows reports no mouse and no mouse buttons. The
check dates from an era when a pointing device could genuinely be absent, and it now fails
on machines whose pointer arrives through a device Windows does not count, leaving the game
unable to start at all. Input support decides what the game can be played with, and a
machine with no pointer at all finds nothing to click.
