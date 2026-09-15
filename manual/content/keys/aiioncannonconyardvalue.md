---
key: AIIonCannonConYardValue
summary: The ion cannon rating a computer house gives an enemy structure that produces buildings.
see_also: [IonCannonDamage, "system:superweapons"]
when_omitted:
  kind: value
  value: ""
---

The rating covers a [`Factory=BuildingType`](/keys/factory/) structure and is the first of the seven structure tests, so a construction yard is never rated as anything else. Like every list in this family it is read at the firing house's own difficulty slot, and must have one entry each for easy, normal and hard. A missing or short list is read past its end. The rating is read only while [the target's strength is at or below `IonCannonDamage`](/systems/superweapons/#the-computers-use). A structure above that figure keeps the flat structure rating of 3.
