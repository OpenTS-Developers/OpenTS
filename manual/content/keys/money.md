---
key: Money
summary: Starting credits for a multiplayer or skirmish match.
see_also: [MaxMoney, MultiplayerAICM]
when_omitted:
  kind: value
  value: "3000"
---

`Money` is where the credits slider starts on the skirmish and network setup screens. When the match begins, every player and computer house the session sets up starts with the amount the slider shows. A computer house then receives an extra share of that money, set by [`MultiplayerAICM`](/keys/multiplayeraicm/). After a player starts a skirmish match, or the host moves the credits slider on the network screen, later setup screens in the same session open at that amount instead.

OpenTS reads the value once, when the program starts. It comes from `RULES.INI`, or from [`MPLAYER.INI`](/formats/multiplayer-rules/) when that file sets it. A map or any other rules layer can set the key, but that does not change the starting money.

Each credits slider runs from `2500` to [`MaxMoney`](/keys/maxmoney/). The skirmish slider moves in steps of `250` and the network slider in steps of `100`, both counted from `2500`. Moving a slider keeps the amount inside that range. Set `Money` to a multiple of `500` in that range so that it lies on the steps of both sliders.

A game started from a [launch file](/formats/spawn-ini/#the-options-every-house-plays-under) ignores `Money` and uses the file's `Credits` instead.
