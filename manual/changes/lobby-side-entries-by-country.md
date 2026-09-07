---
title: Carry the country behind each lobby side entry
category: fix
release: 0.2.0
targets:
- type: key
  id: Side
  scope: multiplayer-settings
  effect: changed
credit: [ZivDero]
---

The skirmish and LAN side boxes now carry each entry's country, so the country chosen is the one
played whatever position it holds in the rules. Both stored the entry's position in the box as
the country, which agreed only while the playable countries were the first two in the rules: a
third playable country played as whatever country sat at its position, and the skirmish box
clamped any remembered choice past the second entry. The LAN player list draws the first side's
icon for a first-side country and the second's for every other, and names a country of a third
side by its own name.
