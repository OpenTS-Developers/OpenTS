---
title: Place health bars from UI.INI
category: feature
release: 0.2.0
breaking: true
migration:
- In the `[Ingame]` section of each `UI.INI` the mod ships, find `InfantryHealthBarPos=` and `UnitHealthBarPos=`.
- Swap the two numbers in each value. OpenTS reads the horizontal offset (X) first, while Vinifera writes the vertical offset (Y) first; for example, `InfantryHealthBarPos=-24,-5` becomes `InfantryHealthBarPos=-5,-24`. The same should be done for `UnitHealthBarPos` as well.
targets:
- type: key
  id: InfantryHealthBarPos
  effect: added
- type: key
  id: UnitHealthBarPos
  effect: added
- type: format
  id: ui-ini
  effect: changed
credit:
- JoyfulShush
- CCHyper
---

Two keys in the `[Ingame]` section of [UI.INI](/formats/ui-ini/#health-bar-positions) set where an object's row of health pips begins: [`InfantryHealthBarPos`](/keys/infantryhealthbarpos/) for infantry and [`UnitHealthBarPos`](/keys/unithealthbarpos/) for vehicles and aircraft. Each takes a horizontal and a vertical offset in pixels; positive values move the row right and down. Their defaults keep the original position.

Credits to CCHyper for their original Vinifera implementation for this feature.
