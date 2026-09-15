---
title: Place starting units in a ring around the start position
category: feature
release: 0.2.0
targets:
- type: system
  id: starting-forces
  effect: changed
credit: [ZivDero, CCHyper]
---

A house's random starting vehicles and infantry are now put down between three and thirty-two cells from its start position. That leaves the cells around the base unit clear, and the units stay where they are put. They were put down from one cell out, and a human player's were then each ordered a cell further away, so a match opened with the units still shuffling around the construction vehicle. The base unit is placed as before, on the start position or the nearest cell that takes it.

The same random seed places every house differently from earlier builds, so a seeded launch no longer reproduces an earlier build's opening layout.

CCHyper is credited for the Vinifera implementation this one follows.
