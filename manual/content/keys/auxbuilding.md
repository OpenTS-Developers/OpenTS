---
key: AuxBuilding
summary: A BuildingType the house must own before a structure may grant this superweapon.
see_also: ["system:superweapons"]
when_omitted:
  kind: value
  value: none
---

The named type is counted across the house's standing structures when a structure offers this weapon [through its own `SuperWeapon=` or `SuperWeapon2=`](/systems/superweapons/#from-a-structure-or-a-plug). A count of zero withholds the weapon, and losing the last one takes it away again. A name matching no BuildingType counts as a type that can never stand, so it withholds the weapon outright; `none` is the spelling that removes the requirement. The stock chem missile names the Nod waste facility this way.

```ini title="rules.ini"
[ChemicalSpecial]
Type=ChemMissile
AuxBuilding=NAWAST   ; Nod Waste Facility
```

:::caution[A plug is never checked against this value]
The upgrade slots of a structure are matched straight against [`SuperWeapon=`](/keys/superweapon/) and [`SuperWeapon2=`](/keys/superweapon2/). The test that a structure's own grant goes through never runs for them. A weapon supplied by a plug ignores this setting both when it is granted and when the removal pass decides whether to keep it. In the stock set the ion cannon and the drop pods arrive only through plugs. The hunter seeker also has a direct grant from the Nod temple, where this setting is tested normally.
:::
