---
title: Read the multiplayer 155mm and ARTYHE values from the rules
category: balance
release: 0.2.0
targets:
- type: key
  id: Verses
  effect: changed
- type: key
  id: ProneDamage
  effect: changed
- type: key
  id: Damage
  effect: changed
- type: key
  id: ROF
  effect: changed
breaking: true
migration:
- 'Add an [`MPLAYER.INI`](/formats/multiplayer-rules/) carrying `[155mm] Damage=115` and `ROF=150` to keep the artillery figures a match used before. Without it the weapon fires at whatever `RULES.INI` gives it, which in stock data is `Damage=150` and `ROF=110`.'
- 'Add `[ARTYHE] Verses=40%,85%,68%,35%,35%` and `ProneDamage=30%` to the same file to keep the artillery warhead. Stock data leaves `Verses=100%,85%,68%,35%,35%` and `ProneDamage=150%` in force outside campaigns, so artillery becomes markedly stronger against unarmored targets and against prone soldiers.'
credit:
- ZivDero
---

The weapon named `155mm` and the warhead named `ARTYHE` no longer have their
figures rewritten in code once a game is not a campaign. Both sections are read
from the rules like every other, so [`Damage`](/keys/damage/),
[`ROF`](/keys/rof/), [`Verses`](/keys/verses/) and
[`ProneDamage`](/keys/pronedamage/) now decide what those two carry in every
game type.

[`MPLAYER.INI`](/formats/multiplayer-rules/) is where the multiplayer figures
belong. A deployment that ships one carrying the values in the migration below
plays exactly as it did.

The warhead's organic flag is derived from its heavy-armor entry, and was
previously computed before the rewrite rather than after it. It now follows the
table in force. Stock `ARTYHE` has a nonzero heavy entry either way, so
the flag does not move for unmodified data.
