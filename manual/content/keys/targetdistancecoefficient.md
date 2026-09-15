---
key: TargetDistanceCoefficient
summary: Weights a candidate target by how far beyond weapon range it lies, and not at all inside it.
see_also: ["system:target-selection"]
when_omitted:
  kind: computed
  note: Takes the value of TargetDistanceCoefficientDefault in [General], which is itself 0 when that key is absent too.
---

The coefficient multiplies the distance from the candidate to the edge of weapon range, and that figure is clamped at zero. The term contributes nothing at all among candidates the object can already shoot. A negative value is what penalizes distance; a positive one rewards it and drives the object toward the farthest candidate it can find. It is read from the type of the object doing the choosing.

Range comes from the weapon this object would choose against that candidate, and falls back to [`GuardRange`](/keys/guardrange/) when no weapon resolves.

:::caution[The penalty is far heavier on a full-map scan]
Range is always counted in cells. Distance is counted in cells on the ring scans used by Guard, Guard Area, Patrol, Move and every building. The full-map scan used by Hunt, Rescue and team attack missions counts distance in leptons (256 to the cell) instead. The same coefficient therefore produces a penalty roughly 256 times larger on the full-map scan than on a ring scan.
:::

:::caution[Zero cannot be pinned on one type]
The value is read with a fallback that substitutes [`TargetDistanceCoefficientDefault`](/keys/targetdistancecoefficientdefault/) whenever the stored figure is zero, and the rules are read again for each later layer. An explicit `0` therefore lasts only until the first later layer that contains the type's section without this key, which puts the global default back.
:::
