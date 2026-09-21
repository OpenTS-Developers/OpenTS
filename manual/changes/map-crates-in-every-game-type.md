---
title: Keep a map's crates in every game type
category: feature
release: 0.2.0
breaking: true
migration:
- Remove the crate overlay from a multiplayer map that should not hand one out. A crate the map draws is now placed in every game type, and switching the match's Crates option off no longer removes it.
targets:
- type: system
  id: crates
  effect: changed
- type: key
  id: Crate
  effect: changed
credit: [ZivDero, Rampastring]
---

Crates a map places in its overlay layer are now kept in skirmish and network games; they used to be removed outside campaigns. The match's Crates option now controls random crates only.
