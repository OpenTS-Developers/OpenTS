---
key: AuxSound2
summary: The sound a structure makes as it starts folding away, and the sound a flying object makes as it comes in to land.
see_also: [AuxSound1, UndeploySound, Locomotor]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[MYHELI] ; an AircraftType registered in [AircraftTypes]
AuxSound2=MYHELI_Landing ; a sound ID registered in SOUND.INI
```

Two unrelated paths read the same slot:

- A **structure** plays it at its own position as a deconstruction begins, a sale or an undeploy, once its animations have stopped. An [`IsJuggernaut=yes`](/keys/isjuggernaut/) structure plays it only after finishing the return of its body and barrel to their start values. The reverse build-up follows it. On a sale that puts out no vehicle, the crew evacuation comes between the two.
- An object flown by the [flyer locomotor](/keys/locomotor/) plays it once per landing, at the ground under it, as it drops below 300 leptons, and only while it still has strength left. A wreck falling out of the sky lands silently.

Nothing else reads the slot, so on a ground vehicle or an infantryman the setting is inert.

:::caution[A structure's `UndeploySound` wins]
[`UndeploySound`](/keys/undeploysound/) is stored in this same slot and is read afterward, so a BuildingType that sets both keeps only `UndeploySound`.
:::

A name that matches no registered sound leaves the previous value in place rather than clearing it.
