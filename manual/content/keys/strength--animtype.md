---
key: Strength
scope: animtype
label: Maximum strength
no_effect: true
see_also: [Crushable, Damage]
when_omitted:
  kind: value
  value: "0"
---

The figure is stored as the type's maximum strength, which the engine reads when something takes damage, when a health ratio is worked out, and when a health ratio is written back. An animation reaches none of them. An explosion's blast sweep gathers its victims from a cell's occupier list, which an animation is never entered into, and from the rosters of aircraft, jumpjet infantry, and [`Jellyfish=yes`](/keys/jellyfish/) vehicles, which it never joins. It is on the logic list, which a heal crate walks without filtering by type. That crate restores only what belongs to the house that opened it, and an animation answers to no house. A vehicle cannot crush an animation: crush victims are taken from the same cell occupier lists the blast sweep reads. Nothing creates an animation with a strength drawn from its type either. No gameplay path reads the figure.

The damage an animation deals to what is underneath it is [`Damage`](/keys/damage/#scope-animtype), which is unrelated to this.
