---
key: Paranoid
summary: Whether the computer houses band together against the human players when the game turns against them.
see_also: [MaxIQLevels, IQ]
when_omitted:
  kind: value
  value: "yes"
---

When the switch is on, two events make the computer houses band together against the human players. Every computer house still in the game allies with every other computer house still in the game, and declares war on every surviving human house. Each alliance this breaks is broken on both sides. Each declaration also adds to the computer house's anger at that human house, which steers the computer's choice of enemy.

Outside a campaign, a [`MultiplayPassive=yes`](/keys/multiplaypassive/) house such as Neutral takes no part in the war declarations. An alliance is also refused when it would leave the computer house with no enemy left in the game.

The two events are:

- A human house forms an alliance with any house that is not [`MultiplayPassive=yes`](/keys/multiplaypassive/), a computer house included. Alliances made while the scenario is being set up, such as those a map or launch file declares, do not count.
- A computer house is defeated while its [`IQ`](/keys/iq/) equals [`MaxIQLevels`](/keys/maxiqlevels/). Every computer player in a skirmish or multiplayer game has that IQ.

With `Paranoid=no`, neither event makes the computer houses band together, and nothing else does.
