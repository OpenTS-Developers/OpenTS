---
title: Count every listed construction yard as one
category: fix
release: 0.2.0
targets:
- type: key
  id: BuildConst
  effect: changed
- type: system
  id: production
  effect: changed
- type: system
  id: superweapons
  effect: changed
- type: system
  id: ai-base-building
  effect: changed
credit: [ZivDero, AlexB]
---

A building of any type listed in `BuildConst` in `rules.ini [AI]` is now a construction yard. It joins its house's yard tally, produces only what the country its house's `ActsLike` names may own, and its loss or capture is judged like entry 0's.

A house whose yard was a later entry was counted as owning none, so a computer house built no structures at all. Capturing one left the placement cursor up, and a second yard type was built for any country at all.

Any vehicle whose `DeploysInto` names a listed yard is likewise an MCV. A computer house sends it to find a site and deploys it, its deploy cursor sits where the yard will, and the ion cannon counts it as an MCV when it picks a target.

A generated base plan starts from the first listed yard the house may own, and treats any listed yard as satisfying a prerequisite. It no longer reads a `BuildPower` entry that names nothing the house may own, or past the end of a base plan with fewer than three entries.

AlexB is credited for the ts-patches bundle that first read this list whole.
