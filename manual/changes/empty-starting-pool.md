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

Rules that set `AllowedToStartInMultiplayer=no` on every InfantryType and every UnitType other than the base unit crashed the match as it set up, because the average price of that empty pool was a division by zero. A house with nothing left to draw — no infantry once two thirds of its budget is spent, or every allowed type above its tech level or not ownable by its country — called through a type it never picked and crashed as well. Such a house now keeps what it has been given, so those rules open a match with each house's base unit alone.

CCHyper is credited for the Vinifera guard on the average price this one follows, and JoyfulShush for the Vinifera guard on the draw.
