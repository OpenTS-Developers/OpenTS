---
title: Read every Tiberium type's damage from the rules
category: fix
release: 0.1.0
targets:
- type: key
  id: Power
  scope: tiberium
  effect: changed
credit: [ZivDero, AlexB, Rampastring]
---

A Tiberium section named `Vinifera` in `rules.ini` now keeps the damage its section states. The value was overwritten with `17` once the section had been read, whatever the rules said, so that one type could not be tuned at all. A Tiberium section that states nothing falls to the same `0` as every other type.
