---
title: Start without asking Windows whether a mouse is attached
category: fix
release: 0.1.0
targets: []
credit: [ZivDero]
---

Startup no longer refuses to run when Windows reports no mouse and no mouse buttons. That check failed on machines whose pointer arrives through a device Windows does not count, leaving the game unable to start at all.
