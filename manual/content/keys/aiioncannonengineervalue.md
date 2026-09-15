---
key: AIIonCannonEngineerValue
summary: The ion cannon rating a computer house gives an enemy engineer.
see_also: [IonCannonDamage, "system:superweapons"]
when_omitted:
  kind: value
  value: ""
---

The rating covers an [`Engineer=yes`](/keys/engineer/) InfantryType and is tested before the [vehicle-thief rating](/keys/aiioncannonthiefvalue/), so a type that is both is rated here. Like every list in this family it is read at the firing house's own difficulty slot, and must have one entry each for easy, normal and hard. A missing or short list is read past its end. The rating is read only while [the target's strength is at or below `IonCannonDamage`](/systems/superweapons/#the-computers-use). An engineer above that figure is left at 1.
