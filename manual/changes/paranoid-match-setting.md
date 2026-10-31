---
title: Choose whether the computers close ranks
category: feature
release: 0.2.0
targets:
- type: key
  id: Paranoid
  effect: changed
- type: format
  id: spawn-ini
  effect: changed
credit:
- fjlaubscher
---

The skirmish Options dialog has a Paranoid box. Checked, the computer houses close ranks against the human players; cleared, they stay on the alliances they already have. The box opens cleared. With `PersistSkirmishSettings=yes` under `[siliconsun]` in `SUN.INI`, the choice is kept in that file's `[Skirmish]` section.

A client launch file sets the same choice with `Paranoid=` under `[Settings]`. Left out, the key means no, and either value replaces the rules key for that match. A campaign, and a network game started from the menu, keep the rules key.
