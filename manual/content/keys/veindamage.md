---
key: VeinDamage
summary: Raw damage a vein cell deals on every second frame.
see_also: ["system:veins", "VeinholeWarhead", "ImmuneToVeins"]
when_omitted:
  kind: value
  value: "2"
---

The figure is raw damage: the [vein attack](/systems/veins/#standing-in-veins) hands it to the object with [`VeinholeWarhead`](/keys/veinholewarhead/). The warhead's own modifier for the object's armor then decides how much is taken off. It is applied on every second frame, and to every unit, structure or infantryman standing in the cell at once rather than to one victim at a time. With no warhead resolved the amount is reduced to nothing whatever this figure says.
