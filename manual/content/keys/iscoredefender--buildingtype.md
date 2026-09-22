---
key: IsCoreDefender
scope: buildingtype
label: Core defender structure
see_also: [DeploysInto, UndeploysInto, "ImmuneToEMP", "system:emp-pulse"]
when_omitted:
  kind: value
  value: "no"
---

The flag is one of the eight that make a structure [one of the deployed-vehicle kinds](/keys/deploysinto/). Such a structure is put down on the deploying vehicle's own cell rather than one cell away and returns to that cell when it undeploys. It may be taken back down whether or not the session allows redeploying.

When the type omits [`ImmuneToEMP`](/keys/immunetoemp/), the flag also makes the structure immune to [EM pulses](/systems/emp-pulse/#what-a-pulse-reaches). A pulse that powers off and stuns other structures leaves it running and only springs its [Paralyzed](/mapping/events/tevent-paralyzed/) trigger event. `ImmuneToEMP=no` removes the immunity.
