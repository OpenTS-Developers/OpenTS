---
key: UnitRepair
summary: Makes the building take in one vehicle or aircraft at a time and repair it for credits.
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: "no"
---

`UnitRepair=yes` makes a building a service depot. It takes in one vehicle or aircraft at a time and repairs it step by step, charging credits for each step. [The repair cycle](/systems/repair/#one-step-at-a-time) covers the steps and their cost.

The flag also has these effects:

- The player's selected vehicles and aircraft get the enter cursor over the depot. A helipad offers that cursor to aircraft without this flag.
- A computer house sends its damaged vehicles to its nearest `UnitRepair` building, under the conditions in [Reaching the pad](/systems/repair/#reaching-the-pad).
- A vehicle or aircraft parked on the depot can be sold where it stands.
- The depot can hold a [rally point](/systems/production/#rally-points), and the object it releases heads there.

The flag is independent of [`RepairBay`](/keys/repairbay/), which names the one building type that repair orders look for. A depot that `RepairBay` does not name still serves whatever reaches it. A `RepairBay` type without this flag refuses the vehicles sent to it.

:::caution[Combine this flag with no other service flag]
A type with this flag and [`UnitReload=yes`](/keys/unitreload/) repairs and never gives the one-point rearming. It still [refills a `ManualReload=yes` object for free](/systems/repair/#what-a-depot-does-for-free). A type with this flag and [`Hospital=yes`](/keys/hospital/) or [`Armory=yes`](/keys/armory/) refuses infantry at the door. [The service order](/systems/repair/#unitreload-is-a-different-service) lists which flag wins.
:::
