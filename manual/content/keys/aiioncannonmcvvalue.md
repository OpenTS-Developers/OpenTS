---
key: AIIonCannonMCVValue
summary: The ion cannon rating a computer house gives an enemy vehicle that deploys into a construction yard.
see_also: [IonCannonDamage, BuildConst, "system:superweapons"]
when_omitted:
  kind: value
  value: ""
---

The test is on the vehicle type's [`DeploysInto`](/keys/deploysinto/) naming any entry of [`BuildConst`](/keys/buildconst/), so a vehicle that deploys into anything else is not covered. Like every list in this family it is read at the firing house's own difficulty slot, and must carry one entry each for easy, normal and hard; a missing or short list is read past its end. The rating is consulted only while [the target's strength is at or below `IonCannonDamage`](/systems/superweapons/#the-computers-use) — a vehicle above that figure is left at 1.
