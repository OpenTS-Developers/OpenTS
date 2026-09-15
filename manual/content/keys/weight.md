---
key: Weight
summary: How strongly an object resists being rocked, and how heavily it presses on ice.
when_omitted:
  kind: value
  value: "1"
---

A blast from a warhead marked [`Rocker`](/keys/rocker/) tips the objects near it, and this figure divides the force. The tilt is scaled by `(0.04 - distance × 0.000025) × force ÷ Weight`, taking the distance in leptons. A scale below `0.01` is dropped and one above `0.05` is held there. The forward component of the tilt then takes half of the scale, and the sideways component all of it. A heavier object therefore shrugs off a jolt that would rock a lighter one, and past the cap a lighter figure buys no more tilt. Only an object drawn from a voxel model is rocked at all, so a type drawn from shape art is never rocked whatever its figure.

In the snow theater the figure is also weighed against two rules-wide thresholds each time a vehicle finishes entering a cell. A vehicle reaching [`IceBreakingWeight`](/keys/icebreakingweight/) breaks the ice under it, and one that only reaches [`IceCrackingWeight`](/keys/icecrackingweight/) cracks it. Where the ice gives way the vehicle starts sinking and is stunned; cracking alone leaves it to drive on. Both tests run on vehicles alone.

Both thresholds are met on equality. The stock figures put the harvester at `1` and most tanks at `3.5`, against a cracking threshold of `2` and a breaking threshold of `4`. A harvester therefore crosses stock ice untouched and a tank cracks it. The civilian recreational vehicle at `4` is the only stock type heavy enough to go through.
