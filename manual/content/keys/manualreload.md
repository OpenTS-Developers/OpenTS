---
key: ManualReload
summary: Stops the type from arming its reload delay when it fires, and has a service depot refill its magazine for free.
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: "no"
---

A vehicle firing normally arms a 450-frame reload delay. The same delay is the pacer behind [`NoMovingFire=yes`](/keys/nomovingfire/): a stationary vehicle gains one ammo point every 450 frames. This flag suppresses only the arming that firing does. What takes its place is the service depot, which fills the magazine to full in a single exchange and charges nothing for it.

The refill pre-empts the repair step for that exchange, so a damaged carrier of this flag rearms first and starts repairing afterwards. An undamaged one is still admitted rather than turned away: it is rearmed on the spot and released. [What a depot does for free](/systems/repair/#what-a-depot-does-for-free) traces the order.
