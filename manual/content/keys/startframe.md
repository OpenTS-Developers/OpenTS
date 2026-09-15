---
key: StartFrame
summary: Parsed frame index that the engine never uses.
no_effect: true
see_also: ["NumLoopFrames", "StartStateAI"]
when_omitted:
  kind: value
  value: "0"
---

The frame a particle draws is chosen by its animation state. A `Gas`, `WeakGas`, `Smoke` or `Web` sequence opens at the frame [`StartStateAI`](/keys/startstateai/) names, and a [`Fire`](/keys/behaveslike/#scope-particletype) particle opens at that state offset by the artwork bank its firing direction picks. A `Spark` or `Railgun` particle draws no artwork at all. No drawing path reads the figure stored here.
