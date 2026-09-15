---
key: AIIonCannonBaseDefenseValue
summary: The ion cannon rating a computer house gives an enemy IsBaseDefense structure.
see_also: [IonCannonDamage, "system:superweapons"]
when_omitted:
  kind: value
  value: ""
---

The rating is the fourth of the seven structure tests, so a base defense that also produces something, or that supplies net power, is rated by one of those tests instead. Like every list in this family it is read at the firing house's own difficulty slot, and must have one entry each for easy, normal and hard. A missing or short list is read past its end. The rating is read only while [the target's strength is at or below `IonCannonDamage`](/systems/superweapons/#the-computers-use). A structure above that figure keeps the flat structure rating of 3.
