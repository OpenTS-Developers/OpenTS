---
key: RadarEventRotationSpeed
summary: Radians a radar event's box turns each frame.
see_also: ["system:map-visibility", RadarEventSpeed, RadarEventMinRadius]
when_omitted:
  kind: value
  value: ".1"
---

Every [radar event](/reference/enums/radar-event/) starts with this turn step and spins its box by the current step each frame. While the box is still closing in, the step stays at the configured value, and [`RadarEventSpeed`](/keys/radareventspeed/) sets how long that sweep lasts. The default of a tenth of a radian is a little under six degrees a frame, a full turn in about sixty-three frames. One degree a frame is about `.017`. Once the radius has reached [`RadarEventMinRadius`](/keys/radareventminradius/), each further frame that fails to settle the box turns it and then cuts the step. Each cut is two percent of the configured value, and the step never falls below a third of it. A box that has to come round again therefore turns more slowly each lap.

The box stops turning on the first frame at that radius when it stands less than the current step past the orientation it opened in. A quarter turn counts as that same orientation, because the box is square. It then turns by that leftover angle once more and comes to rest. That frame is also when [`RadarEventDurations`](/keys/radareventdurations/) and [`RadarEventVisibilityDurations`](/keys/radareventvisibilitydurations/) are loaded, and only an event that has stopped turning is ever removed.

:::caution[A non-positive step never settles]
At zero the box never turns, so the angle past that orientation stays a hair above zero and the test can never pass. Its timers are never loaded and the event is never removed. It goes on being drawn for the rest of the match, and for the three suppressible kinds goes on swallowing later events of its kind. A negative step still settles, because it drives that angle negative and the test passes as soon as the angle falls below the step.
:::
