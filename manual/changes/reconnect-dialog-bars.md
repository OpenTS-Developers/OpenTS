---
title: Draw every player's connection bar in its own box
category: fix
release: 0.2.0
targets:
- type: system
  id: reconnect-dialog
  effect: changed
credit:
- ZivDero
---

The reconnect dialog's table of bar controls named the first player's box twice, so the
second player's bar was drawn over the first player's and the second box stayed empty. The
original game has the same fault. Each bar now draws in its own box.
