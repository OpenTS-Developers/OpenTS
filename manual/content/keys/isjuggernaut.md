---
key: IsJuggernaut
summary: Marks a deployed structure as a Juggernaut, which stows its gun before it packs up.
see_also: [DeploysInto, UndeploysInto, StartPitch, StartFacing]
when_omitted:
  kind: value
  value: "no"
---

The flag is one of the eight that make a structure [one of the deployed-vehicle kinds](/keys/deploysinto/). Such a structure is put down on the deploying vehicle's own cell rather than one cell away and returns to that cell when it [undeploys](/keys/undeploysinto/). It may be taken back down whether or not the session allows redeploying, and [an EM pulse](/systems/emp-pulse/#what-a-pulse-reaches) attaches sparks to it as it is stunned.

Two effects belong to this flag alone, and both are about stowing the gun. Deconstruction is held at its first step until the barrel pitch has come back to [`StartPitch`](/keys/startpitch/) and the body to [`StartFacing`](/keys/startfacing/), so the structure keeps turning and nothing else in the sequence begins until it arrives. The vehicle produced by the undeploy is then created with its barrel already at `StartPitch`.

A computer-owned Juggernaut also packs itself up when it is handed a target beyond the range of a primary weapon that is not anti-aircraft. The target is dropped, and the structure is sent straight into deconstruction unless it is immobilized. [`TickTank=yes`](/keys/ticktank/) and [`Artillary=yes`](/keys/artillary/) structures share that.
