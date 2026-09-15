---
key: ChargeToDrainRatio
summary: The exchange rate between a charge-draining superweapon's stored charge and the time its effect runs.
see_also: [UseChargeDrain, RechargeTime, "system:superweapons"]
when_omitted:
  kind: value
  value: "3"
---

A [`UseChargeDrain=yes`](/keys/usechargedrain/) weapon spends its charge while its effect is running, and this figure converts between the two. Firing a fully charged weapon puts a drain of [`RechargeTime`](/keys/rechargetime/) multiplied by this value on the clock. Firing it again divides whatever drain is left by the same value and turns it back into charge, so switching the effect off part-way returns the matching share of the charge. The same product is what the cameo clock is drawn against while the effect runs.

A ratio below 1 makes the effect shorter than the charge that bought it: the shipped `rules.ini` sets `.333`, which spends the whole three-minute firestorm charge on one minute of wall. Above 1 it runs longer than the charge took to build. One figure in `[General]` covers every charge-draining weapon; [the firestorm generator's charge](/systems/laser-fences/#charge-and-drain) sets out both formulas as the wall uses them.
