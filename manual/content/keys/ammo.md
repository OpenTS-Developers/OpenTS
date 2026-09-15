---
key: Ammo
summary: The number of shots a runtime instance holds, or the number of visitors a hospital or armory can admit.
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: "-1"
---

An instance is created with a full pool. A count of exactly zero is what refuses a shot, and firing only counts a positive pool down, so the default of `-1` is an unlimited supply rather than an empty one.

A building refills its pool the instant it empties. A [`Hospital=yes`](/keys/hospital/) or [`Armory=yes`](/keys/armory/) building is the exception: its pool is spent one point per visitor admitted and is never restocked. On those two types the value is the number of visits the building will ever serve, and leaving it unset gives exactly one. [Hospitals and armories](/systems/repair/#hospitals-and-armories) traces the admission.

A vehicle or aircraft that has run its pool down is refilled at a [`UnitReload=yes`](/keys/unitreload/) building one point at a time, or in one free step at a service depot if it is [`ManualReload=yes`](/keys/manualreload/).
