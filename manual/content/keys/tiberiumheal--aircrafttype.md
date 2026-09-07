---
key: TiberiumHeal
scope: aircrafttype
label: Heals on Tiberium
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: "no"
---

While the object occupies a Tiberium cell and is below full strength, it regains its type's repair step — [`IRepairStep`](/keys/irepairstep/) for infantry, [`RepairStep`](/keys/repairstep/) for everything else — each time the [global interval](/keys/tiberiumheal/#scope-global-rules) elapses, and the last step is clamped to full strength. Unlike [`SelfHealing=yes`](/keys/selfhealing/), which stops at the yellow line until a [`SelfHealCap`](/keys/selfhealcap/) says otherwise, this restores an object completely with no setting asked. Only infantry, vehicles and aircraft heal this way; a building's flag never reaches the healing path. The `TIBERIUM_HEAL` ability from [`VeteranAbilities`](/keys/veteranabilities/) or [`EliteAbilities`](/keys/eliteabilities/) reaches the same path.

The flag has a second effect on every object type, buildings included: an object destroyed with it set spews first-growth Tiberium into five cells around it — north-west, north, east, south and west. Its own cell takes none. The north-west corner is reached rather than the centre because the list opens with the no-direction marker, which the step that reads it folds round to that corner instead of standing still.
