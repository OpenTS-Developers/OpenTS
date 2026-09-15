---
key: Artillary
summary: Marks a deployed structure as artillery, which faces north when it digs in and stows its gun before it packs up.
see_also: [DeploysInto, UndeploysInto, StartPitch, StartFacing, TurretAnimIsVoxel]
when_omitted:
  kind: value
  value: "no"
---

The flag is one of the eight that make a structure [one of the deployed-vehicle kinds](/keys/deploysinto/). Such a structure is put down on the deploying vehicle's own cell rather than one cell away, and it returns to that cell when it undeploys. It may be taken back down whether or not the session allows redeploying, and [an EM pulse](/systems/emp-pulse/#what-a-pulse-reaches) that stuns it attaches sparks as well.

Four effects follow from this flag.

- The deploy facing is north, and no other kind uses that facing. A vehicle holds its deploy order until it has turned north, and the vehicle produced by a later undeploy is created facing north.
- The structure is created with its barrel pitch level to the east, which a [`TickTank=yes`](/keys/ticktank/) structure also gets.
- The undeploy is held at its last step, after the deconstruction animation has run, until the barrel pitch has come back to [`StartPitch`](/keys/startpitch/) and the body to [`StartFacing`](/keys/startfacing/). The vehicle is then created with its barrel already at `StartPitch`. An [`IsJuggernaut=yes`](/keys/isjuggernaut/) structure runs the same wait at its first step instead.
- A structure drawing its turret from a voxel model keeps drawing it at all times. Every other [`TurretAnimIsVoxel=yes`](/keys/turretanimisvoxel/) structure hides the turret while its build-up animation runs and again once deconstruction is past its first frame.

A computer-owned artillery structure also packs itself up when it is handed a target beyond the range of a primary weapon that is not anti-aircraft. The target is dropped and the structure is sent straight into deconstruction, unless it is immobilized. `TickTank=yes` and `IsJuggernaut=yes` structures share that.

One reader sits on the vehicle rather than the structure. A human player's vehicle whose [`DeploysInto`](/keys/deploysinto/) names an `Artillary=yes` structure never retaliates on its own.

:::caution[The key is spelled `Artillary`]
The engine looks for that spelling alone. `Artillery=` is never read, and a section with it leaves the structure unflagged.
:::
