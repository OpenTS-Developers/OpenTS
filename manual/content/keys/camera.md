---
key: Camera
summary: Marks an aircraft with the weapon as a loaner rather than as the player's own.
see_also: ["Landable", "Selectable", "Primary"]
when_omitted:
  kind: value
  value: "no"
---

The name promises a weapon that photographs the ground and reveals the area around whatever fired it. No path in the engine performs that reveal, and firing such a weapon is in every way an ordinary shot. The flag reaches exactly one live decision.

That decision is taken as an aircraft enters the map. An aircraft whose first weapon slot holds a camera weapon is marked a loaner, the same mark that [`Landable=no`](/keys/landable/) and [`Selectable=no`](/keys/selectable/#scope-aircrafttype) leave, and any one of the three is enough. A loaner cannot be selected by the player, is allowed to leave the map, and is put on the Retreat mission where an aircraft the player keeps would be told to guard. Nothing comes of that last one. An aircraft handles the Retreat mission itself, not the way everything else on the map does, and its handling does nothing at all. A loaner told to retreat stays exactly where it was.

```ini title="rules.ini"
[MySpyCamera] ; example WeaponType
Camera=yes
```

Only the first weapon slot is read, and only on an aircraft. The same weapon in a second slot, or on any other kind of object, changes nothing at all.
