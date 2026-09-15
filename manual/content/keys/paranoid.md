---
key: Paranoid
summary: Whether the computer houses close ranks against the human players when the game turns against them.
see_also: [MaxIQLevels, IQ]
when_omitted:
  kind: value
  value: "yes"
---

Two events read the switch, and both do the same thing. Every computer house that is still in the game allies with every other computer house that is still in the game, and declares every surviving human house an enemy. Alliances broken this way are broken on both sides, and each such declaration stokes the anger that decides which house a computer treats as its target.

- A human house forms an alliance with a house that is not multiplayer-passive, an alliance with a computer house included. The check does not run while the scenario is being set up, so alliances a map or a launch file declares in advance never set it off.
- A computer house is defeated while holding exactly [`MaxIQLevels`](/keys/maxiqlevels/) as its [`IQ`](/keys/iq/), which outside a campaign game is the level every computer house is given.

Setting this to `no` leaves both events alone; nothing else in a played match bands the computer houses together. Replaying a recording is the exception: a player leaving the recorded session hands that house to the computer and bands the computer houses together on the spot, without reading the switch.

:::caution[The entry is spelled `Paranoid`]
`ComputerParanoid` is not read anywhere. Only `Paranoid` in `[AI]` reaches this switch, and a rules file spelling it any other way leaves the behavior on.
:::
