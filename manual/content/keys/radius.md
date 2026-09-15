---
key: Radius
summary: How far a smoke system throws the two successors of an expiring particle.
see_also: ["NextParticle", "NextParticleOffset"]
when_omitted:
  kind: value
  value: "0"
---

A smoke system replaces each expiring particle with a pair of [`NextParticle`](/keys/nextparticle/) successors thrown out on opposite sides of it. That is what makes a plume widen as it rises instead of staying a column. This figure sets how far: the X and Y displacements are each drawn between an eighth and a quarter of it, in leptons, and one successor takes the displacement while the other takes its mirror image. The two are always placed symmetrically, so the pair straddles the point the original died at.

Nothing else reads the figure. Gas, weak gas and web systems place their single successor with [`NextParticleOffset`](/keys/nextparticleoffset/) instead, fire, spark and railgun systems create no successors, and no part of a particle's own drawing, collision or damage reads it.

:::danger[A radius of 0 through 7 stops the game]
The displacement is drawn in multiples of an eighth of this figure, taken in whole numbers, which is what lands it between the eighth and the quarter named above. Anything from `0` to `7` makes that eighth zero, and the game stops the moment a particle of the type expires in a smoke system. A type held by a smoke system needs at least `8` here as soon as it names a successor, and a type that never mentions the key sits at `0`. Negative values divide cleanly and do not fault.
:::
