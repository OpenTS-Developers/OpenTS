---
key: IsMobileWar
summary: Marks a deployed structure as a mobile war factory, which an engineer may work on and which may pack up at any time.
see_also: ["system:capture"]
when_omitted:
  kind: value
  value: "no"
---

A structure that names an [`UndeploysInto`](/keys/undeploysinto/) type is otherwise treated as a vehicle and left out of the engineer's cursor entirely (a construction yard excepted, which is carved out of that rule already). This setting covers both requirements at once: the structure counts as not a deployed vehicle, and as [`Repairable=yes`](/keys/repairable/). A mobile war factory therefore offers the engineer's cursor whether or not it is repairable.

:::caution[The cursor promises a restore and delivers a capture]
The structure still counts as a vehicle at the moment the engineer arrives, and [that branch](/systems/capture/#the-vehicle-branch) tests neither ownership nor [`Capturable`](/keys/capturable/). An allied mobile war factory below full strength offers the engineer's repair cursor, and the resulting order changes its owner instead.
:::

Three smaller effects follow the setting. The structure may undeploy in circumstances that would otherwise forbid it. Band-box selection passes it over as it does a construction yard. A vehicle thief chasing the vehicle loses its target outright when that vehicle deploys; every other object chasing it is handed the new structure instead. Opening for business also toggles the structure's primary-factory state for its production kind.
