---
key: SpotlightMovementRadius
summary: Distance in leptons behind a structure at which its spotlight's sweep pivots, and the range beyond which a followed target is dropped.
see_also: [SpotlightLocationRadius, SpotlightRadius, HasSpotlight]
when_omitted:
  kind: value
  value: "2000"
---

The point a sweeping beam turns about sits this far behind its structure. It lies along the direction the structure faces, opposite the aim point placed by [`SpotlightLocationRadius`](/keys/spotlightlocationradius/). The beam therefore swings on an arc whose radius is the two figures added together, a little under twelve cells with both keys left at their defaults. A structure casts a beam only where its type sets [`HasSpotlight=yes`](/keys/hasspotlight/).

```ini title="rules.ini"
[General]
SpotlightMovementRadius=2560  ; the sweep pivots ten cells behind the structure
```

The span between the two radii is graded in tenths, and every tenth the beam has traveled out from its structure widens what it can see. [`SpotlightRadius`](/keys/spotlightradius/) covers that widening. Raising this figure therefore does three things at once: it widens the swing, stretches each tenth of the grading, and holds a followed target further out. No other key sets the follow range on its own.

A beam [set to follow a target](/mapping/actions/taction-change-spotlight-behavior/) holds onto it only while the target is nearer to the structure than this figure.

A following beam reverts to sweeping the moment it has no target: none was found when the behavior was taken up, the target was destroyed, or the target passed this distance. Its sweep restarts from the middle of the arc rather than from where the target had led it. The intruder scan resumes on that same frame, while the beam still stands where it left off.
