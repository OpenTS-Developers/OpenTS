---
key: Repairable
summary: Allows a structure to be repaired with the wrench, worked on by an engineer, and treated as a building by a commando.
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: "yes"
  note: "InfantryTypes start at `no`, though no path reads an infantry's value."
---

Only a BuildingType's value is ever read. The computer's own [repair-or-sell decision](/systems/repair/#when-the-computer-repairs) runs through the same candidate test, so `Repairable=no` also exempts a building from automatic repair and sell-back. The cursor consumers:

- The wrench accepts the building only while this is set and the building is below full [`Strength`](/keys/strength/#scope-aircrafttype), or while it has a limpet mine.
- An engineer offers no restore cursor on an allied structure without it, unless the type is a mobile war factory. His capture cursor over an enemy structure needs this key set too, along with [`Capturable=yes`](/keys/capturable/).
- A commando plants explosives on a building with it and shoots at one without it.

A service depot never reads the value, so `Repairable=no` on a vehicle or aircraft does not stop a depot from repairing it. The wrench, meanwhile, refuses every non-building whatever the value says.
