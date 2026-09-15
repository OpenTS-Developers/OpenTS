---
key: Velocity
summary: The speed a particle is created with, in leptons per frame.
see_also: ["Deacc", "BehavesLike", "XVelocity", "MinZVelocity"]
when_omitted:
  kind: value
  value: "0"
---

What the speed moves depends on the [behavior](/keys/behaveslike/#scope-particletype). A `Fire` particle covers this many leptons along its line of fire each frame, at anything from `0.72` to `1.08` times this figure. A flame given no speed at all is flagged for death on its first logic frame, before it has traveled anywhere. A `Smoke` particle climbs by this many leptons a frame, with a random one lepton added or taken away as it is created so a plume does not rise as a solid column. A `Railgun` particle steps this far along the beam each frame, after the holding system has added its own perturbation to each particle it lays down. `Gas`, `WeakGas`, `Spark` and `Web` particles hold a speed but never move by it: a spark's motion comes from [`XVelocity`](/keys/xvelocity/), [`YVelocity`](/keys/yvelocity/) and [`ZVelocityRange`](/keys/zvelocityrange/) instead, and a web particle does not move at all.

A cell is 256 leptons across, so the stock flame's `28.0` crosses a cell in nine frames while a railgun particle's `0.4` barely creeps. [`Deacc`](/keys/deacc/) takes the speed back down from here, and a successor created through [`NextParticle`](/keys/nextparticle/) inherits the speed its predecessor had reached rather than starting again at this figure.
