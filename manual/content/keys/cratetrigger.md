---
key: CrateTrigger
summary: Whether collecting the overlay springs the crate pickup trigger events.
see_also: ["system:crates"]
when_omitted:
  kind: value
  value: "no"
---

Collecting the overlay springs [Pickup Crate](/mapping/events/tevent-pickup-crate/) on the tag attached to the collector, and raises the scenario flag that fires [Pickup Crate (any)](/mapping/events/tevent-pickup-crate-any/) for every general trigger on the following logic pass. Both happen before a result is chosen. When the object-attached trigger destroys the collector, the sequence stops there: the crate stays on the map and no result is delivered.

The flag is independent of [`Crate=yes`](/keys/crate/), which the engine reads separately, but an overlay that is not a crate is never collected, so the trigger never springs without it.
