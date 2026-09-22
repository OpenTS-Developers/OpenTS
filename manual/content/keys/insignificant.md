---
key: Insignificant
summary: Whether the object is left out of its house's inventory counts, loss announcements and responses to attack.
---

An `Insignificant=yes` object is left out of the following:

- **The house's own inventory.** It is neither added nor removed as it appears and dies, so it never reaches the running counts a house keeps of what it owns.
- **The building loss and kill records.** A structure marked this way is not counted against its owner as a building lost, is not credited to whoever destroyed it, and does not redraw the multiplayer score display.
- **The loss announcement.** When a vehicle, soldier or aircraft marked this way dies, EVA does not announce the loss and the [Goto Radar Event](/commands/centeronradarevent/) cell does not move. Structures never make [this announcement](/systems/destruction-and-debris/#the-loss-announcement).
- **The response to being attacked.** Damaging it raises neither the owner's base-attack response nor the warning the player hears. A barrel or a mine can therefore be shot at without the house treating it as an attack on its base. For the warning, a structure that counts as a vehicle is passed over on that ground instead. A structure counts as a vehicle when its type sets [`UndeploysInto=`](/keys/undeploysinto/) and is not a construction yard.
- **The computer's defensive response.** A computer house sends no defensive detachment when the object is attacked.

```ini title="rules.ini"
[MYBARREL]     ; an example BuildingType
Insignificant=yes
```

:::caution[Units still count as losses and kills]
Only structures are left out of the loss and kill records. An `Insignificant=yes` vehicle, soldier or aircraft dies unannounced, but it still adds to its house's count of units lost and still counts as a unit kill for the house that destroyed it.
:::

Where the setting starts depends on what is reading it. An object a house can own begins at `no`, while the types that are never owned (animations, projectiles, overlays, smudges, terrain and voxel animations) begin at `yes`. Those types read the same key through the same shared step, so an assignment on one of them is accepted and stored. No behavior reads the value back, so the assignment changes nothing either way.
