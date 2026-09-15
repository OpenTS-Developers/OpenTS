---
key: BridgeStrength
summary: How hard a bridge is to bring down, and the damage a demolition charge does to one.
see_also: ["DestroyableBridges", "IonCannonWarhead", "C4Warhead"]
when_omitted:
  kind: value
  value: "1000"
---

A blast that can destroy walls rolls once against this figure for each bridge span it reaches. A whole number is picked between one and the figure, and the span takes damage when that number lands below the blast's raw damage. A smaller figure therefore makes bridges easier to bring down, and a figure of `1` makes any blast of two points or more damage every span it touches. None of it happens at all unless [`DestroyableBridges`](/keys/destroyablebridges/) is on and the warhead sets [`Wall=yes`](/keys/wall/#scope-warheadtype).

A blast with [`IonCannonWarhead`](/keys/ioncannonwarhead/) skips the roll outright. At a tile bridge (the road and rail spans that carry traffic over water), it is also given up to four attempts at the span, retrying until one of them lands. Every other warhead gets one. The extra attempts stop there: at a low bridge the ion cannon skips the roll and then lands the same two hits every other warhead lands.

The same figure is the raw damage of the charge an infantryman leaves when it demolishes a bridge from the deck. Three blasts of it go off at the sapper's feet, all with [`C4Warhead`](/keys/c4warhead/): the first is credited to the sapper, the other two to nobody. Raising the figure to make bridges harder to shell therefore also makes that charge deadlier to everything standing nearby.

:::caution[A low bridge is damaged twice for one roll]
The overlay bridges that carry a road across a gully are damaged, then damaged again, on a single successful roll. Only the first result decides whether the span counts as destroyed, but both hits land.
:::
