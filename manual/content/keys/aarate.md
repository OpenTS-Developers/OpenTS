---
key: AARate
summary: The servicing delay an armed building on this mission waits between passes.
see_also: [Rate, SAM, HasStupidGuardMode]
when_omitted:
  kind: inherited
  note: The mission's own Rate=, taken from the same section.
---

The value is a fraction of a minute, multiplied out to game frames at 900 to the minute and truncated, with a random zero to two frames added on top. Only two paths ask for it, and both belong to structures.

- An armed building idling on guard, having found nothing to attack, waits this long before looking again.
- A [`SAM=yes`](/keys/sam/) building on the attack mission reaches it only as a fall-through. Both states of its launcher return a single frame, so the delay is served only when the mission was entered as an override. An override does not restart the mission, so the launcher's state machine stays wherever the previous mission left it.

```ini title="rules.ini"
[Guard]
Rate=.030
AARate=.016
```

The name, and the note beside it in the shipped rules, describe an anti-aircraft override. Neither path tests whether the building or its target has anything to do with aircraft: this is the ordinary servicing delay for a weapon-equipped structure standing on guard.

This key never times an unarmed building. [`HasStupidGuardMode`](/keys/hasstupidguardmode/) defaults to `yes`, which holds such a building's guard pass on a fixed 100-frame cycle that reads neither figure. A type that turns the flag off is serviced at three times the mission's [`Rate`](/keys/rate/#scope-mission-behavior), or at `Rate` itself for a repair facility. `Rate` reaches a building on its own account too: the repair, missile, unload and open missions all read it with no test of armament.

:::caution[The delay can come out at zero frames]
The read falls back to the mission's `Rate=` whenever the value read is zero, so `AARate=0` behaves exactly like leaving the key out. A section that also sets `Rate=0` takes that zero through the fallback. A figure below about `.0012` multiplies out to zero once truncated, leaving nothing but the random frames the caller adds on top.
:::
