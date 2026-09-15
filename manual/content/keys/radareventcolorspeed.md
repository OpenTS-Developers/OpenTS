---
key: RadarEventColorSpeed
summary: Fraction of the blend between a radar event's two colors covered each frame.
see_also: ["system:map-visibility"]
when_omitted:
  kind: value
  value: ".05"
---

A [radar event](/reference/enums/radar-event/) pulses between two colors fixed by its kind, and this value is how far along that blend it moves each frame. The direction reverses at either end, so the pulse runs back and forth rather than jumping. At the default a sweep from one color to the other takes twenty frames. A there-and-back cycle takes forty, a little under three seconds.

The same value scales the gradient stroked along each edge of the box, so a faster pulse also lays more color bands around the outline. At zero the event holds the color it opened with and never pulses. A negative value flips sign on its first frame, so the event then pulses exactly as it would with that value written positive.
