---
key: AIIonCannonAPCValue
summary: The ion cannon rating a computer house gives an enemy vehicle that can carry passengers.
see_also: [IonCannonDamage, "system:superweapons"]
when_omitted:
  kind: value
  value: ""
---

The test is on the vehicle type's [`Passengers`](/keys/passengers/) being above zero, whether or not it is carrying any at the time, and is the last of the three vehicle tests; anything else is rated a flat 2. Like every list in this family it is read at the firing house's own difficulty slot, and must have one entry each for easy, normal and hard. A missing or short list is read past its end. The rating is read only while [the target's strength is at or below `IonCannonDamage`](/systems/superweapons/#the-computers-use). A transport above that figure is left at 1.
