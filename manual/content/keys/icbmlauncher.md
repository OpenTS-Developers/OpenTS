---
key: ICBMLauncher
summary: Marks a deployed structure as an ICBM launcher, which faces east when it digs in.
see_also: [DeploysInto, UndeploysInto, SuperWeapon]
when_omitted:
  kind: value
  value: "no"
---

The flag is one of the eight that make a structure [one of the deployed-vehicle kinds](/keys/deploysinto/). Such a structure is put down on the deploying vehicle's own cell rather than one cell away and returns to that cell when it undeploys. It may be taken back down whether or not the session allows redeploying, and [an EM pulse](/systems/emp-pulse/#what-a-pulse-reaches) attaches sparks to it as it is stunned.

One effect belongs to this flag alone: the deploy facing is east, so a vehicle holds its deploy order until it has turned east and the vehicle produced by a later undeploy is created facing east. [`SensorArray=yes`](/keys/sensorarray/) and [`TickTank=yes`](/keys/ticktank/) structures deploy at the same facing.

Nothing about launching a missile follows from the flag. A structure fires a superweapon because [`SuperWeapon=`](/keys/superweapon/) names one, and [superweapons](/systems/superweapons/) covers what that grants and how it is aimed.
