---
key: RechargeTime
summary: The minutes a superweapon takes to charge, and for a charge-draining weapon the base the drain is scaled from.
see_also: ["system:superweapons"]
when_omitted:
  kind: value
  value: "5"
---

The figure is in minutes and is converted to game frames at 900 frames to the minute. Fractions are accepted: the stock chem missile uses `.3`. A [`UseChargeDrain=yes`](/keys/usechargedrain/) weapon measures its discharged spell from the same figure, scaled by [`ChargeToDrainRatio`](/keys/chargetodrainratio/), so raising the delay lengthens both the charge and the effect.

```ini title="rules.ini"
[MyIonStrike] ; example superweapon section
Type=IonCannon
RechargeTime=8.5
```

:::caution[`RechargeTime=0` is read as if the key were absent]
The read takes no default from the value already in force. A missing key and a written `0` both fail the same test, and both leave the delay standing at whatever the last file to set it left there, five minutes if no file ever has. An instantly recharging superweapon cannot be authored here, and a rules layer cannot use `0` to undo a delay an earlier layer set.
:::
