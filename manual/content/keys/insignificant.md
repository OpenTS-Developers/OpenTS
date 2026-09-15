---
key: Insignificant
summary: Whether the object is kept out of its house's inventory tallies and its response to being attacked.
---

An object marked this way is passed over in four places.

- **The house's own inventory.** It is neither added nor removed as it appears and dies, so it never reaches the running counts a house keeps of what it owns.
- **The building loss and kill records.** A structure marked this way is not counted against its owner as a building lost, is not credited to whoever destroyed it, and does not redraw the multiplayer score display.
- **The response to being attacked.** Damaging it raises neither the owner's base-attack response nor the warning the player hears. A barrel or a mine can therefore be shot at without the house treating it as an attack on its base. For the warning, a structure that counts as a vehicle is passed over on that ground instead. A structure counts as a vehicle when its type sets [`UndeploysInto=`](/keys/undeploysinto/) and is not a construction yard.
- **The computer's defensive response.** A computer house sends no defensive detachment when the object is attacked.

```ini title="rules.ini"
[MYBARREL]     ; an example BuildingType
Insignificant=yes
```

:::caution[A unit marked this way is still counted as a unit lost]
Only the building record is guarded. A vehicle, a soldier or an aircraft with this setting still adds to its house's tally of units lost when it dies, and still credits a unit kill to whatever destroyed it. Structures alone are kept out of both records.
:::

Where the setting starts depends on what is reading it. An object a house can own begins at `no`, while the types that are never owned (animations, projectiles, overlays, smudges, terrain and voxel animations) begin at `yes`. Those types read the same key through the same shared step, so an assignment on one of them is accepted and stored. No behavior reads the value back, so the assignment changes nothing either way.
