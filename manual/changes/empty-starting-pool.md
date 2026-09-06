---
title: Stop drawing starting units when nothing is left to draw
category: fix
release: 0.2.0
targets:
- type: key
  id: AllowedToStartInMultiplayer
  effect: changed
credit: [ZivDero, CCHyper, JoyfulShush]
---

A house that runs out of types to draw for its starting units now keeps what it has been
given. Rules that set `AllowedToStartInMultiplayer=no` on every InfantryType and every
UnitType other than the base unit open a match with each house's base unit alone; the
average price of that empty pool was a division by zero. A house with no infantry to draw
once two thirds of its budget is spent, or with nothing to draw at all because every allowed
type is above its tech level or not ownable by its country, stops there; it called through a
type it never picked, and the game crashed as the match set up.

CCHyper is credited for the Vinifera guard on the average price this one follows, and JoyfulShush
for the Vinifera guard on the draw.
