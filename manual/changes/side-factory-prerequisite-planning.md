---
title: Let a base plan satisfy a side factory prerequisite
category: fix
release: 0.2.0
targets:
- type: key
  id: PrerequisiteGDIFactory
  effect: changed
- type: key
  id: PrerequisiteNodFactory
  effect: changed
- type: system
  id: ai-base-building
  effect: changed
credit: [ZivDero]
---

A computer house assembling its base plan now counts a `GDIFACTORY` or `NODFACTORY` prerequisite
as met once any type of the matching `PrerequisiteGDIFactory` or `PrerequisiteNodFactory` list
is queued, as production already did for a house playing. The planner had no answer for either
group and treated both as never satisfied, so a structure naming one was left out of every
generated plan.
