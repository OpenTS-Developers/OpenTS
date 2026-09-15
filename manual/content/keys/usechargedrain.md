---
key: UseChargeDrain
summary: Whether the superweapon holds its effect for a timed spell instead of firing once.
see_also: ["system:superweapons", "system:power"]
when_omitted:
  kind: value
  value: "no"
---

A charge-draining weapon runs [three states instead of two](/systems/superweapons/#charge-draining-weapons): charging, ready, and discharged. Firing it while ready puts it into the discharged state and turns the clock into a countdown of how long the effect lasts, scaled by [`ChargeToDrainRatio`](/keys/chargetodrainratio/). Firing it again returns it to ready with the ratio applied the other way. The discharged state itself does nothing: the [`Type=`](/keys/type/#scope-superweapontype) effect goes out once, with the click that starts the drain, and firing again mid-drain only ends the drain early. Letting the drain run out returns it to charging with a full [`RechargeTime`](/keys/rechargetime/) on the clock and takes the effect down.

The flag governs only the state machine and the cameo. What the weapon actually does when it is fired still comes from [`Type=`](/keys/type/#scope-superweapontype), and the [firestorm defense](/systems/laser-fences/#the-firestorm-generator-and-its-charge) is the one behavior whose raise and lower calls this state machine drives.

The cameo follows from the same flag. Such a weapon always reports itself as charging, so it always draws a clock and never shows the plain ready face. It can be fired from any state but charging or suspended.

:::caution[Suspension costs a charge-draining weapon its progress]
Where an ordinary weapon resumes from the point its timer stopped, this one has its timer reset to a full `RechargeTime` when it comes back. Every spell of [low power](/systems/power/#superweapons), however brief, therefore starts its charge over, and [`IsPowered=no`](/keys/ispowered/) is the only way to keep it charging through one.
:::
