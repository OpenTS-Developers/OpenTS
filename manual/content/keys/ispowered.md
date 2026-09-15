---
key: IsPowered
summary: Whether the superweapon stops charging while its house is short of power.
see_also: ["system:power"]
when_omitted:
  kind: value
  value: "yes"
---

A superweapon is suspended when the building granting it is powered down (its on/off switch off) or its house cannot meet its drain. This value is what the suspend step checks, so `IsPowered=no` leaves the weapon charging through both. Resuming does not read the value, so a weapon suspended for another reason still comes back. One path bypasses the check: a weapon granted while the house is already short of power arrives suspended whatever this value says, and resumes once the building is on and the power restored.

:::caution[Suspension costs a charge-draining weapon its progress]
An ordinary superweapon's timer is stopped where it stands and restarts from there. A weapon declared [`UseChargeDrain=yes`](/keys/usechargedrain/) instead has its timer reset to a full [`RechargeTime`](/keys/rechargetime/) when it resumes, so every spell of low power starts its charge over. `IsPowered=no` is the only way to keep such a weapon charging through one.
:::
