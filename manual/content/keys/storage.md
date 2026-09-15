---
key: Storage
summary: Units of Tiberium an object can hold.
see_also: ["system:tiberium", "Harvester", "SiloDamage", "Value"]
when_omitted:
  kind: value
  value: "0"
---

A harvester fills to this figure in whole growth stages and heads home when it reaches it; a harvester type left at `0` never lifts a stage at all. A stock harvester sets `Storage=28`, so 28 stages fill it, and at first-growth's shipped `Value` of 25 a full load is worth 700 credits when the house spends it. It is equally the ceiling a [weeder](/systems/veins/) fills to. A building instead adds the figure to its house's capacity while it stands, and incoming loads fill each storage building in turn until its figure is reached. Capturing the building moves its capacity and its contents to the new owner. Destroying it scatters the contents across the surrounding ground, and selling it offers them back to whatever storage the house has left; [credits and storage](/systems/tiberium/#credits-and-storage) covers all three. Anything a house harvests beyond its combined capacity is thrown away rather than converted to credits.

The store counts units, not credits. Each unit is tagged with the Tiberium type it came from and priced at that type's [`Value`](/keys/value/) when the house spends it. A mixed load keeps its per-type worth all the way to the purchase.
