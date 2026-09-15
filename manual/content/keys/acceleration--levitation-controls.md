---
key: Acceleration
scope: levitation-controls
label: Levitation thrust
see_also: ["AccelerationDuration", "InitialBoost", "Drag", "AccelerationProbability"]
when_omitted:
  kind: value
  value: "0.5"
---

The figure is added to a levitating unit's velocity on each frame of a thrust, in the direction the thrust was aimed, for [`AccelerationDuration`](/keys/accelerationduration/) frames. It is in leptons per frame gained per frame (256 leptons to a cell, 15 frames to the second). A figure of `1` adds one lepton to the unit's speed on each frame of a thrust. [`Drag`](/keys/drag/) is subtracted on each of those frames too, so the speed a thrust reaches is exact: the [`InitialBoost`](/keys/initialboost/) it opens with plus `AccelerationDuration × (Acceleration − Drag)`. The stock figures reach `1.5 + 20 × (0.5 − 0.05)`, or 10.5 leptons per frame. The sum holds while the whole velocity points one way; a thrust opened over older motion on another heading adds vectors, and only a frame-by-frame vector sum gives its speed then.

Nothing caps the result. [`MaxVelocityWhenFollowing`](/keys/maxvelocitywhenfollowing/) and [`MaxVelocityWhenPissedOff`](/keys/maxvelocitywhenpissedoff/) decide when a coast ends, and [`MaxVelocityWhenHappy`](/keys/maxvelocitywhenhappy/) decides when a fresh thrust may begin. None of them decides how fast a thrust may leave the unit traveling, so a large figure here raises the unit's top speed without limit. [`IntentionalDriftVelocity`](/keys/intentionaldriftvelocity/) describes what happens once one frame's movement crosses more than a single cell.

[`Drag`](/keys/drag/) covers which objects read this section and the `[General]` section a file must contain for any of it to be read at all.
