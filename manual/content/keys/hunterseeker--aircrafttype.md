---
key: HunterSeeker
scope: aircrafttype
label: Hunter-seeker drone flag
see_also: ["system:ion-storms"]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[MyHunterDrone] ; example AircraftType
HunterSeeker=yes
```

Makes the type a hunter-seeker drone that acquires its own target, moves straight at it, and is never grounded by an ion storm. The firing check never reports a shot as ready for such a type, so it does not open fire however it is armed. Its movement heads straight at its target instead of routing around what stands between them, and a flying hunter seeker with no target acquires one for itself rather than landing.

:::note[Hunter seekers fly through ion storms]
The flying locomotor reports itself ion-sensitive for every type except this one, so a hunter-seeker aircraft is the one kind that neither loses power nor crashes when [a storm breaks](/systems/ion-storms/#the-storm-breaks). The flag has no bearing on a type that uses any other locomotor.
:::
