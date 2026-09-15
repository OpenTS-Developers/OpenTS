---
key: UnitRepair
summary: Makes the building take in one vehicle or aircraft at a time and repair it for credits.
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: "no"
---

The flag makes a building answer a docking request from a vehicle or an aircraft, and offers a selected player-controlled vehicle or aircraft the enter cursor onto its pad. It then runs [the repair cycle](/systems/repair/#one-step-at-a-time). A helipad also offers that cursor to an aircraft without this flag. `UnitRepair` is what a computer house's damaged vehicles look for when they go to be repaired, and what makes a docked object sellable where it stands.

It is read independently of [`RepairBay`](/keys/repairbay/), which names the one type a repair order steers toward. A building with this flag that `RepairBay` does not name still serves whatever reaches it; a `RepairBay` type without this flag refuses every docking request it receives.

:::caution[A building runs only its first service branch]
The servicing mission tests construction yard, then [`Hospital`](/keys/hospital/), then [`Armory`](/keys/armory/), then this flag, then [`UnitReload`](/keys/unitreload/), and stops at the first one the type sets. The docking answer runs in a different order and tests this flag before the two infantry flags, `Hospital=` and `Armory=`. A type that is both a depot and a hospital therefore turns infantry away at the door.
:::
