---
key: InitialBoost
summary: The speed a levitating unit gains at once when a thrust begins.
see_also: ["Acceleration", "AccelerationDuration", "Drag", "IntentionalDriftVelocity"]
when_omitted:
  kind: value
  value: "1.5"
---

The figure is added to the unit's velocity on the frame a thrust starts, in the direction the thrust is aimed, before any of the per-frame [`Acceleration`](/keys/acceleration/) is applied. It adds to whatever velocity the unit already holds instead of replacing it, so a thrust taken while the unit is moving compounds with the motion it had. The figure is in leptons per frame (256 leptons to a cell, 15 frames to the second). A figure of `8` starts a thrust at eight leptons per frame, about half a cell per second before [`Drag`](/keys/drag/) wears it off.

It is applied on every thrust, whether the unit is wandering at random or heading for something. When [`AccelerationDuration`](/keys/accelerationduration/) is zero, it is the whole of a thrust's effect. Nothing caps the result; [`IntentionalDriftVelocity`](/keys/intentionaldriftvelocity/) describes what happens once one frame's movement crosses more than a single cell.

[`Drag`](/keys/drag/) covers which objects read this section and the `[General]` section a file must contain for any of it to be read at all.
