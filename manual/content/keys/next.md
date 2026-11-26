---
key: Next
summary: The animation this one turns into instead of finishing.
see_also: ["LoopCount", "Start", "Surface", "Rate", "End"]
when_omitted:
  kind: value
  value: "none"
---

When its last pass ends, the animation switches to the named type and keeps playing. It stays the same animation, at the same position and attached to the same object, so an explosion that settles into a column of smoke is one animation from start to finish.

From the switch on, the named type's [`End`](/keys/end/), [`LoopCount`](/keys/loopcount/), [`Rate`](/keys/rate/), [`RandomRate`](/keys/randomrate/) and [`Normalized`](/keys/normalized/#scope-animtype) settings apply. The named type's start effects run as though the animation had just begun, including its [`Report=`](/keys/report/#scope-animtype) sound.

The animation follows the chain one link each time it reaches an end. Two types that name each other keep one animation alive indefinitely.

A name that `[Animations]` does not list still creates an animation type, and that type is read from its `art.ini` section like a listed one. If `art.ini` has no section for it either, the type has no stages, and an animation that switches to it ends at its next frame advance.

## What the change of type does not carry over

- The animation restarts at stage number `Start` of the new type, not at frame `Start`. Frames are drawn at `Start` plus the stage, so a new type whose [`Start`](/keys/start/) is not `0` opens on the frame numbered twice its `Start`.
- The animation keeps the layer and height it was created with. The new type's [`Surface=`](/keys/surface/) is not applied.
- The new type's [`Reverse=yes`](/keys/reverse/) does not change the direction, and the animation keeps stepping the way it already was. The new type's `Reverse` setting still decides where its passes end and restart, so chain only between types that both leave `Reverse` unset.
