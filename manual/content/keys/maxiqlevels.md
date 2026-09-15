---
key: MaxIQLevels
summary: The top of the house intelligence scale, which computer houses hold outside campaign games.
see_also: [IQ]
when_omitted:
  kind: value
  value: "5"
---

The value caps what a scenario may write into a house's [`IQ`](/keys/iq/): a larger figure there is replaced by `1`, not by this ceiling. Outside campaign games every computer house is set to exactly this level when it is created or taken over. The behaviors the `[IQ]` thresholds gate are therefore all open for it as long as they sit at or below this value.

The level also marks the trigger for computer paranoia. When a computer house holding exactly this value is defeated in a multiplayer game and [`Paranoid=yes`](/keys/paranoid/) allows it, every surviving computer house allies with the other computers and turns hostile to every human player.
