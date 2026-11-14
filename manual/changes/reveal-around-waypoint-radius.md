---
title: Set the reveal radius and height test from the action line
category: feature
release: 0.2.0
targets:
- type: action
  id: TACTION_REVEAL_SOME
  effect: changed
- type: key
  id: RevealTriggerRadius
  effect: changed
- type: system
  id: map-visibility
  effect: changed
credit: 
- JoyfulShush
---

Maps can now set the radius and turn off the height test on each `Reveal around waypoint` action. The radius, which is denoted by a number of cells to reveal, goes in the first of the four fields after the waypoint number; `0` or less falls back to `[General] RevealTriggerRadius` in `rules.ini`. A non-zero second field skips the height test, so ground behind high ground is revealed too, and leaving both fields at `0` changes nothing.
