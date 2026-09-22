---
key: ChainReaction
summary: Whether damage to the overlay's cell can set off the Tiberium standing there.
see_also: ["system:tiberium", "Tiberium", "Power"]
when_omitted:
  kind: value
  value: "no"
---

The detonation is drawn from the Tiberium in the cell rather than from the overlay itself. It runs only when that cell holds Tiberium at stage 2 or higher, counting from stage 0: see [Damage](/systems/tiberium/#damage). For a [`Tiberium=yes`](/keys/tiberium/#scope-overlaytype) overlay the triggering warhead must also declare [`Tiberium=yes`](/keys/tiberium/#scope-warheadtype). A sonic wave sets the reaction off with no warhead check at all. A chain-reactive overlay that is not Tiberium does not detonate on the flag alone; the reaction needs Tiberium in the cell. A Tiberium cell also loses one growth stage for every ten points of damage that reached it, whether or not the detonation itself went off. An overlay with both this flag and [`Explodes=yes`](/keys/explodes/#scope-overlaytype) runs both on the same hit: the reaction is judged first, and the explosive detonation follows it.
