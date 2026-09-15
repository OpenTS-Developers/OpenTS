---
title: Normalize nonpositive weapon bursts
category: fix
release: 0.1.0
targets:
- type: key
  id: Burst
  effect: changed
credit: [ZivDero, dkeeton]
---

A WeaponType whose `Burst` is zero or negative now stores one when its rules section is read. The firing and animation paths therefore never receive zero as their burst divisor from rules data.
