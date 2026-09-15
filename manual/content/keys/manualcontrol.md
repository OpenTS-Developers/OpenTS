---
key: ManualControl
summary: Whether the superweapon's charge timer is left stopped instead of counting down on its own.
see_also: ["system:superweapons"]
when_omitted:
  kind: value
  value: "no"
---

The timer is set to a full [`RechargeTime`](/keys/rechargetime/) and then stopped when the weapon is granted, and stopped again after every discharge. The weapon therefore never charges by itself. [Something outside the timer has to start it](/systems/superweapons/#manual-control), and the engine has exactly one such starter: a [`Type=ChemMissile`](/keys/type/) weapon begins charging when its house's [weed pool](/systems/veins/#the-weed-pool) reaches [`WeedCapacity`](/keys/weedcapacity/). A weapon of any other behavior declared `ManualControl=yes` therefore stays at zero charge for the whole match.

Because the timer is stopped rather than running, clicking the cameo before the weapon is charged speaks [`SuspendVoice=`](/keys/suspendvoice/) rather than [`ImpatientVoice=`](/keys/impatientvoice/).
