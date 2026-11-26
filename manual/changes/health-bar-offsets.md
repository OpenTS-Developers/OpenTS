---
title: Place health bars from UI.INI
category: feature
release: 0.3.0
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

The keys have Vinifera's names, but Vinifera reads the vertical offset first, so swap the two numbers in a Vinifera file. CCHyper is credited for the Vinifera implementation this follows.
