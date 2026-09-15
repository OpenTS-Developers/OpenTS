---
key: Money
summary: Starting credits for a multiplayer or skirmish match.
see_also: [MaxMoney, MultiplayerAICM]
when_omitted:
  kind: value
  value: "3000"
---

The value seeds the match's starting-credits option as the rules are read, and the setup screen then writes over it. Every house a non-campaign session sets up for a player or a computer opponent is opened with the resulting figure, and a computer house is then given a further share of that figure, which [`MultiplayerAICM`](/keys/multiplayeraicm/) sets by difficulty.

Every setup screen that offers the figure holds it on a slider running from a fixed `2500` up to [`MaxMoney`](/keys/maxmoney/). The screen hands back whatever the slider is left showing, so the seeded figure survives only where it already sits inside that range.
