---
key: SellSound
summary: Sound played as a structure, a unit or a wall section is sold back.
see_also: [CrumbleSound, GenericClick, RefundPercent, Unsellable]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
SellSound=SELL1 ; a sound ID registered in SOUND.INI
```

Three sales play it.

A structure beginning its build-down plays it from its own position, so it fades with distance from the view, and only for a player-controlled house. A structure that undeploys into a vehicle is torn down without it; a construction yard, which undeploys but is not treated as a vehicle, still plays it.

A vehicle or aircraft sold back while it stands on a building plays it at full volume rather than from a place on the map. It again plays only for a player-controlled house, alongside the spoken "unit sold" line. A vehicle or aircraft standing in the open cannot be sold at all, and infantry are never sold; the sell click does nothing to them. Selling a repair bay also sells the unit docked on it, which gets the same sound and spoken line.

A wall section sold back plays it at full volume, and only when the selling house is the local player's own. Wall removal has a silent form as well, and that is the default. A section cleared to make room for a structure being placed over it takes the silent form, and so do the walls of a defeated house being swept up. The wall sale the player orders takes the loud one. The money-raising sell-off the AI runs against its own base takes the loud form too, but the sound still requires the selling house to be the local player's own, so the player never hears it.
