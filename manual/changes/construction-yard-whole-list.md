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

A building of any type listed in `BuildConst` is now a construction yard: it joins its house's
yard tally, produces only for the country its own record names, and its loss or capture is
judged like entry 0's. A house whose yard was a later entry owned none by that tally, so a
computer house built no structures at all, a captured yard left the placement cursor up, and a
second yard type built for any country at all. A vehicle that deploys into any
listed yard is likewise an MCV: a computer house sends it to find a site and deploys it, its
deploy cursor sits where the yard will, and the ion cannon rates it as one. A generated base
plan starts from the first listed yard the house may own, treats any listed yard as satisfying
a prerequisite, and no longer reads a missing power plant or a queue of fewer than three
entries past its end.

AlexB is credited for the ts-patches bundle that first read this list whole.
