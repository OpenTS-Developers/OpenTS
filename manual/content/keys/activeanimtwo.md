---
key: ActiveAnimTwo
summary: The animation the structure runs in its second active slot.
see_also: ["ActiveAnim", "ActiveAnimTwoDamaged", "ActiveAnimTwoX", "ActiveAnimTwoY", "ActiveAnimTwoYSort", "ActiveAnimTwoZAdjust", "ActiveAnimTwoPowered", "ActiveAnimTwoPoweredLight", "TurretAnimIsExclusive"]
when_omitted:
  kind: value
  value: ""
---

[Building animations](/systems/building-animations/) covers the slot mechanism all four share. The value is one animation's name; a comma list matches nothing. The second slot is the one a charging turret takes over. On a [`TurretAnimIsExclusive=yes`](/keys/turretanimisexclusive/) structure the slot's animation is destroyed as the turret starts charging, and it does not run while the turret is charging or charged. It is created again once the charge is spent. A charging turret on a structure without that flag leaves the slot alone: both animations run at once.

Installing an [upgrade](/keys/upgrades/) is the exception. A plug that brings a damaged structure back to maximum strength restarts every empty active slot without checking the turret. An exclusive turret that is charging at that moment ends up running this slot alongside its own animation until the charge is spent.

How often the slot's animation repeats is the animation's own art.ini setting, [`LoopCount`](/keys/loopcount/): the slot asks for one pass, and that figure decides how many the animation makes. When the passes run out the animation ends or chains, and a slot whose animation has ended sits empty until an event restarts it.
