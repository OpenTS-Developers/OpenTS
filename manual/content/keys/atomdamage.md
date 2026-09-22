---
key: AtomDamage
summary: Raw damage of a wide-area nuclear blast that nothing in play reaches.
no_effect: true
see_also: ["NukeWarhead", "MaxDamage", "ExpSpread"]
when_omitted:
  kind: value
  value: "1000"
---

The blast the figure describes is centered on the point of detonation and uses [`[SpecialWeapons] NukeWarhead`](/keys/nukewarhead/). In a campaign it reaches four cells and uses the figure as written; in a skirmish or multiplayer match it reaches three cells and one fifth of the figure, rounded down. Nothing is credited with the damage.

The figure is the raw damage handed to the same wide-area routine an exploding object's death blast uses. That routine scales each cell's share by the cell's distance from ground zero, and the share rises toward the rim, as [`ExpSpread`](/keys/expspread/) describes. The center cell and the rim cells take the full figure or more, and the cells between them take less.

It is not the damage an ordinary nuclear shot delivers. A detonation with the nuclear warhead does its damage the way every other shot does, through its own warhead. This wide-area blast sits behind a test of whether the explosion animation object was allocated at all, a test the allocator never fails, so nothing in play reaches it.
