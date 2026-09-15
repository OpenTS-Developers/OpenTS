---
key: NoAutoFire
summary: Stops a human-owned object of this type from picking targets on its own.
see_also: ["system:target-selection"]
when_omitted:
  kind: value
  value: "no"
---

Every automatic target scan by an object of this type ends immediately, with no target, while the object belongs to a human-controlled house. A computer house never reads the setting, so the same type scans normally in computer hands.

Only the scan is suppressed. A target given by a player order, by a team script or by the retaliation path is unaffected, and the object still fires at whatever it has been handed.
