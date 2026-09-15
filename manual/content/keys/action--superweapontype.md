---
key: Action
scope: superweapontype
label: Superweapon cursor action
see_also: ["system:superweapons"]
when_omitted:
  kind: value
  value: "None"
---

Any value but `None` makes clicking the charged cameo arm targeting mode. The cursor over the map then reports this action in place of the ordinary one, and releasing the button fires the weapon at the cell under it. `None` skips targeting altogether and [discharges the weapon straight from the cameo click](/systems/superweapons/#aiming-and-the-click) at cell 0,0, which is how the firestorm defense and the hunter seeker are fired.

This key and [`Type=`](/keys/type/#scope-superweapontype) are read independently. `Action=` only chooses the cursor and which click fires the weapon; `Type=` alone decides what the weapon does when it fires, so a section whose two disagree works as each key says.

The values that give a superweapon cursor are `Nuke`, `IonCannon`, `DropPod`, `ChemBomb`, `EMPulse` and `EMPulseRange`. The last two are the in-range and out-of-range forms, and the [EM pulse](/systems/emp-pulse/#em-pulse-cannon-superweapon) picks between them for itself.

:::caution[An unrecognized value reads as `None`]
A misspelling is not rejected. It resolves to `None`, and the weapon then fires from the cameo click at cell 0,0 with no targeting step and no choice of where the effect lands.
:::

:::caution[The map click finds the weapon by this value alone]
Every left click that resolves to an action searches the declared superweapons in `[SuperWeaponTypes]` order for the first one with that `Action=`, whether or not targeting mode was armed. Two sections sharing a value therefore always fire the earlier of the two. A value that ordinary orders also produce (an attack, a move) discharges a charged weapon on the next such order.
:::
