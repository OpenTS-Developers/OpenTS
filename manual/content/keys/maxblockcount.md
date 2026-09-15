---
key: MaxBlockCount
summary: Parsed retry count that decides nothing.
no_effect: true
see_also: ["IntentionalDriftVelocity", "ProximityDistance"]
when_omitted:
  kind: value
  value: "4"
---

A levitating unit that cannot enter the cell it is moving into tries once more at a point nudged by a single lepton, and where that also fails it spends one of these tries. The figure refills the count when the tries run out and again on every successful move, so a running tally of consecutive blocked frames is kept.

Nothing turns on the tally. Running out clears the unit's destination, but only on the condition that it has neither a live target nor a live destination. Reaching that condition is itself what clears the destination, so the step has nothing left to do. The behavior is the same at every value. A blocked unit drifts back to the middle of its own cell at [`IntentionalDriftVelocity`](/keys/intentionaldriftvelocity/) and departs again in a fresh direction, however long the obstruction has held it.

A newly created unit starts on four tries whatever the figure says, and picks the figure up from its first successful move onward.

[`Drag`](/keys/drag/) covers which objects read this section and the `[General]` section a file must contain for any of it to be read at all.
