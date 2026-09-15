---
key: ColorSpeed
summary: How fast a spark or railgun particle slides from one of its colors to the next.
see_also: ["ColorList", "StartColor1", "StartColor2"]
when_omitted:
  kind: value
  value: "0"
---

The particle holds a fraction between the two [`ColorList`](/keys/colorlist/) entries it is currently mixing. Every frame this figure plus a fresh random amount below `0.05` is added to that fraction. When the fraction passes 1, the particle steps to the next pair of entries and starts again at 0. A rate of `0.13` therefore steps roughly every six or seven frames, and `0.01` roughly every thirty. The random part alone takes the particle through a step about every forty frames, which is what a type that names no rate gets.

Only the two pixel-drawn behaviors, [`Spark` and `Railgun`](/keys/behaveslike/#scope-particletype), advance the fraction. The random component is drawn per particle per frame, so no two particles of a burst reach the same color together even when they were created on the same frame.

A negative rate large enough to cancel the random part never reaches the step, and the fraction instead runs below zero without a floor. The mix is then extrapolated backwards past the first color of the pair rather than held at it, and the particle is drawn in colors that appear nowhere in its list.
