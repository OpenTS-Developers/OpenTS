---
key: MaxIQLevels
summary: The top of the house intelligence scale, which computer opponents hold outside campaign games.
see_also: [IQ]
when_omitted:
  kind: value
  value: "5"
---

Outside a campaign game, every computer opponent plays at exactly this [`IQ`](/keys/iq/) level. That covers the computer players set up for the match and a player's house that the computer takes over after the player leaves. Every `[IQ]` threshold at or below this value is therefore open to those houses, and every threshold above it stays closed.

A scenario's `IQ=` for a house is limited to this value. A larger figure is replaced by `1`, not by this value.

The level also triggers computer paranoia. When a computer house at exactly this IQ is defeated and [`Paranoid=yes`](/keys/paranoid/), every surviving computer house that did not start the match allied with a human house still in the game allies with the other such computer houses and turns hostile to every human player. In a campaign, this happens only when the [Destroy all of...](/mapping/actions/taction-house-destroy-all/) trigger action defeats a house whose `IQ=` equals this value.
