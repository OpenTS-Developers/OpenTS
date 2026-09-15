---
key: IsLimpetMine
summary: Marks a deployed structure as a limpet mine, which vehicles drive over and an EM pulse destroys.
see_also: [DeploysInto, UndeploysInto, EMPulseCannon, IsMobileWar, "system:emp-pulse"]
when_omitted:
  kind: value
  value: "no"
---

The flag is one of the eight that make a structure [one of the deployed-vehicle kinds](/keys/deploysinto/). Such a structure is put down on the deploying vehicle's own cell rather than one cell away, returns to that cell when it undeploys, and may be taken back down whether or not the session allows redeploying.

Four further effects follow from it.

- A vehicle is never blocked by one. The mine is skipped by the test a vehicle runs before entering a cell, the same way an [`InvisibleInGame=yes`](/keys/invisibleingame/) structure is, so traffic drives straight over it.
- A selected limpet mine offers no attack cursor over anything, so a player cannot force fire it. The suppression is the one an [`EMPulseCannon=yes`](/keys/empulsecannon/) structure gets: no cursor over an object and none over a bare cell.
- [An EM pulse destroys it outright](/systems/emp-pulse/#what-a-pulse-reaches) with the firing house credited, where every other structure in the blast is only powered off and stunned.
- The [Deploy Object](/commands/deployobject/) command accepts it. Every other structure fails that command's readiness test unless it sets [`Passengers=`](/keys/passengers/) of its own; this flag and [`IsMobileWar=yes`](/keys/ismobilewar/) clear it outright.
