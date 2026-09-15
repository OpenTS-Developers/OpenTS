---
key: AIIonCannonHarvesterValue
summary: The ion cannon rating a computer house gives an enemy harvesting vehicle.
see_also: [IonCannonDamage, "system:superweapons"]
when_omitted:
  kind: value
  value: ""
---

The rating covers a [`Harvester=yes`](/keys/harvester/) UnitType and is the first of the three vehicle tests, so a harvester is never rated as a deployer or a transport. Like every list in this family it is read at the firing house's own difficulty slot, and must have one entry each for easy, normal and hard. A missing or short list is read past its end. The rating is read only while [the target's strength is at or below `IonCannonDamage`](/systems/superweapons/#the-computers-use). A harvester above that figure is left at 1.
