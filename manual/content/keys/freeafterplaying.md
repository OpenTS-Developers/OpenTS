---
key: FreeAfterPlaying
summary: Gives a demand-loaded animation's artwork back as soon as an animation of the type leaves the game.
see_also: ["DemandLoad", "Image", "Next"]
when_omitted:
  kind: value
  value: "no"
---

The flag is read only on a [`DemandLoad=yes`](/keys/demandload/#scope-animtype) animation; on any other type it parses and does nothing, because there is nothing the type is holding on its own account to give back.

Where it does apply, the artwork is released as the animation object is destroyed — whether it played to its end, was cut short with the object it was pinned to, or was thrown away when a structure stopped it. The type then reads the shape from disk again the next time anything draws an animation of it, so the pair suits a rarely played animation and costs a disk read per playing on a common one.

An animation that has chained through [`Next=`](/keys/next/) releases the artwork of the type it was holding when it was destroyed, not the type it started as, so the earlier types in a chain keep theirs for the rest of the scenario.

[Animation shape](/keys/demandload/#scope-animtype) covers shape ownership and loading.
