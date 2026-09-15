---
key: TiberiumExplosive
scope: global-rules
label: Exploding harvesters
see_also: ["system:tiberium", "Power", "Storage"]
when_omitted:
  kind: value
  value: "no"
---

A unit destroyed while holding any Tiberium adds a second blast to its normal explosion. The blast reaches one and a half cells out from where the unit died, is attributed to the destroyed unit, and is applied through [`C4Warhead`](/keys/c4warhead/). It deals the sum, across every Tiberium compartment (one per Tiberium type carried), of the amount stored multiplied by that type's [`Power`](/keys/power/#scope-tiberium). A unit type with no death explosion of its own never blasts, whatever it carries: the normal explosion comes from the type's `Explosion=` list, or from its `ScrapExplosion=` list while [`ScrapMetal`](/keys/scrapmetal/) is on. A scenario running with the [harvester truce](/keys/harvesterimmune/) option set skips the blast.
