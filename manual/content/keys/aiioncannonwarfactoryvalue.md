---
key: AIIonCannonWarFactoryValue
summary: The ion cannon rating a computer house gives an enemy structure that produces vehicles.
see_also: [IonCannonDamage, "system:superweapons"]
when_omitted:
  kind: value
  value: ""
---

The rating applies to a [`Factory=UnitType`](/keys/factory/) structure. It is the second of the seven structure tests, so it wins over every test below it: a war factory that is also a base defense is rated here. Like the other ion cannon rating lists, it is read at the firing house's own difficulty slot, so it needs one entry each for easy, normal and hard. A missing or short list is read past its end. The rating is read only while [the target's strength is at or below `IonCannonDamage`](/systems/superweapons/#the-computers-use). A structure above that figure keeps the flat structure rating of 3.
