---
key: SAM
summary: Runs the structure on an anti-air firing routine of its own that holds only airborne targets.
see_also: [AA, Powered, "system:power", "system:target-selection"]
when_omitted:
  kind: value
  value: "no"
---

Two paths read the flag, and between them they replace the ordinary firing behavior of a defense.

Every pass of the structure's logic drops its current target when that target is not in the air. Nothing else is consulted — not range, not whether the weapon could have hit it — so a launcher handed a ground target by [target selection](/systems/target-selection/) or by retaliation loses it again before it acts on it. A player cannot hand it one: the attack cursor appears over an airborne aircraft in range and nowhere else, so a launcher takes an order against a chosen aircraft and none against the ground.

On the attack mission the structure then runs a two-state routine in place of the general one:

1. **Tracking.** It stalls while its house is short of power, on [the test the defense gate uses](/systems/power/#defenses). It drops the target unless that target is an aircraft standing above ground level. Otherwise it turns toward the target and moves to firing once it is within 45 degrees of the aim direction.
2. **Firing.** It re-tests the target the same way, then asks its first weapon slot whether the shot is clear. An illegal, impossible or out-of-range shot drops the target and returns it to tracking, and a facing refusal returns it without dropping the target; any other refusal — reloading above all — leaves it in the firing state, re-testing every frame — and a clear shot fires the first and second weapon slots in the same pass before returning to tracking.

Two things the general attack path does are missing from that routine: nothing chooses between the weapon slots, and there is no allowance for snapping a turret onto a target it is nearly facing. Each pass also schedules the next one frame later, so a launcher on the attack mission is serviced every frame.

:::caution[The flag does not make the weapon anti-air]
Whether a shot may be taken at an airborne target is decided by [`AA=yes`](/keys/aa/) on the projectile. A launcher whose weapon lacks it drops every ground target it is given and is then refused every air target it turns to.
:::
