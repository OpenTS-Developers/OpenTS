---
key: TickTank
summary: Marks a deployed structure as a tick tank, which faces east and levels its gun as it digs in.
see_also: [DeploysInto, UndeploysInto, DeployToFire]
when_omitted:
  kind: value
  value: "no"
---

The flag is one of the eight that make a structure [one of the deployed-vehicle kinds](/keys/deploysinto/). Such a structure is put down on the deploying vehicle's own cell rather than one cell away, and it returns to that cell when it undeploys. It may be taken back down whether or not the session allows redeploying, and [an EM pulse](/systems/emp-pulse/#what-a-pulse-reaches) attaches sparks to it as it is stunned.

Three effects follow from this flag.

- The deploy facing is east. A vehicle holds its deploy order until it has turned east, and the vehicle produced by a later undeploy is created facing east as well.
- The structure is created with its barrel pitch level to the east, which an [`Artillary=yes`](/keys/artillary/) structure also gets.
- A computer-controlled vehicle whose [`DeploysInto`](/keys/deploysinto/) names one digs in before firing, but only against another vehicle and only where its own cell could take a building. Against infantry or a structure it stays mobile. [`DeployToFire=yes`](/keys/deploytofire/) on the vehicle overrides all of that and makes it dig in for every target.

A computer-owned tick tank also packs itself up when the deployed structure is handed a target beyond the range of its own primary weapon, if that weapon is not anti-aircraft. The target is dropped and the structure is sent straight into deconstruction, unless it is immobilized. [`Artillary=yes`](/keys/artillary/) and [`IsJuggernaut=yes`](/keys/isjuggernaut/) structures share that.
