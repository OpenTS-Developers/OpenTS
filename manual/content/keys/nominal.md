---
key: Nominal
summary: Shows the type's real name to a player it is not allied with, instead of a generic label.
see_also: [Technician, Crewed, Invisible]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[MYPRISON] ; a BuildingType registered in [BuildingTypes]
Nominal=yes
```

Holding the cursor over an object shows its name. That is normally the type's [`Name=`](/keys/name/), though a technician (below) and a [disguised](/keys/disguise/) soldier can show another name. The player sees the name on objects that the player or an ally owns, and an [observer](/systems/observers/) sees it on every object. Any other object normally shows a generic label instead: "Enemy Soldier" for infantry, "Enemy Vehicle" for a vehicle or aircraft, and "Enemy Structure" for a structure. `Nominal=yes` exempts the type, so a civilian or story structure shows its real name to every player.

A structure placed by a map can carry the same exemption on that one building, whatever its type sets.

On an InfantryType, the key has a second, unrelated effect. A survivor of this type can be marked as a technician. A marked survivor is named with the game's "Technician" text, cannot be picked up as a civilian evacuee, and is not counted among its house's infantry. A sold structure marks every such survivor; a destroyed structure marks them only if it has build-up artwork; a vehicle's escaping crew is never marked.

The mark applies whatever type the survivor is, whether it came from [`Crew`](/keys/crew/), [`Technician`](/keys/technician/) or [`Engineer`](/keys/engineer/).
