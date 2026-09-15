---
key: IsMobileStealth
summary: Marks a deployed structure as a mobile stealth generator, one of the kinds treated as a deployed vehicle.
see_also: [DeploysInto, UndeploysInto, CloakGenerator]
when_omitted:
  kind: value
  value: "no"
---

The flag is one of the eight that make a structure [one of the deployed-vehicle kinds](/keys/deploysinto/). Membership in that set is its whole effect. Such a structure is put down on the deploying vehicle's own cell rather than one cell away and returns to that cell when it undeploys. It may be taken back down whether or not the session allows redeploying, and [an EM pulse](/systems/emp-pulse/#what-a-pulse-reaches) attaches sparks to it as it is stunned.

Unlike the other kinds in that set, this one has nothing of its own: no facing, no gun handling, no cloaking. The field a mobile stealth generator projects comes from [`CloakGenerator=yes`](/keys/cloakgenerator/), which any structure may set, and the type it packs back into from [`UndeploysInto`](/keys/undeploysinto/).
