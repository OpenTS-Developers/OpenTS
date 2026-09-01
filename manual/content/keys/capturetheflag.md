---
key: CaptureTheFlag
summary: Seeds the capture-the-flag session option, which no longer reaches the game.
see_also: [BaseUnit, Bases]
no_effect: true
when_omitted:
  kind: value
  value: "no"
---

As the rules are read, the value seeds a multiplayer session option of the same name, alongside the other `[MultiplayerDefaults]` options. That option is sent to the other machines during setup and a guest takes the host's — but nothing carries it into the scenario flag the game logic consults.

:::caution[The mode cannot be turned on]
The scenario flag starts false and no routine sets it. The one screen that used to write it belonged to modem play and went with it, no scenario file carries the flag, and the routine that would broadcast a change of it is never called.
:::

With the flag on, and with [`Bases`](/keys/bases/) on as well, each house's [`BaseUnit`](/keys/baseunit/) is given that house's own flag as it is placed. Carrying it halves the unit's speed and stops it cloaking — it shimmers instead — and the flag drops back onto the ground when the unit deploys or is otherwise taken off the map. A house that is defeated has its flag taken away.

:::caution[Nothing captures a flag]
No routine moves a flag from one house to another, and no victory condition reads one. Turning the mode on would mark the starting base units and slow them down; the contest the name describes is not in the engine.
:::
