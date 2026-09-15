---
key: AIIonCannonPlugValue
summary: The ion cannon rating a computer house gives an enemy IsPlug structure.
see_also: [IonCannonDamage, "system:superweapons"]
when_omitted:
  kind: value
  value: ""
---

The rating covers a structure with [`IsPlug=yes`](/keys/isplug/), which in the shipped rules is the upgrade host that plugs fit into; a fitted plug is deleted as it is placed and is never itself a target. Like every list in this family it is read at the firing house's own difficulty slot, and must have one entry each for easy, normal and hard. A missing or short list is read past its end. The rating is read only while [the target's strength is at or below `IonCannonDamage`](/systems/superweapons/#the-computers-use). A structure above that figure keeps the flat structure rating of 3.
