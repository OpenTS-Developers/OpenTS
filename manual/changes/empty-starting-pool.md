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

Rules that set `AllowedToStartInMultiplayer=no` on every InfantryType and every UnitType other than the base unit crashed the match as it set up. The average price of that empty pool was a division by zero. A house with nothing left to draw called through a type it never picked and crashed as well. That happened once two thirds of its budget was spent with no infantry left, or when every allowed type sat above the house's tech level or was not ownable by its country. Such a house now keeps what it has been given, so with `Bases=yes` these rules open a match with each house's base unit alone.

CCHyper is credited for the Vinifera guard on the average price this one follows, and JoyfulShush for the Vinifera guard on the draw.
