---
key: End
summary: The number of stages the animation runs through before it ends or chains.
see_also: ["Start", "LoopEnd", "LoopCount", "Next", "Rate"]
when_omitted:
  kind: computed
  note: The frame count of the shape the type first gets hold of, or zero if it never gets one.
---

The animation shows one frame of its shape per stage, starting at [`Start`](/keys/start/). Stages count from zero, so the frames shown are `Start` through `Start` + `End` - 1. When the stage counter reaches `End`, the animation is removed or becomes its [`Next=`](/keys/next/) animation in the same logic pass, and that stage is never drawn.

`End` limits only the last pass. While an animation still has passes left to run, each pass ends at [`LoopEnd`](/keys/loopend/) instead. Without [`LoopCount=`](/keys/loopcount/), the animation makes one pass.

`End` can run past the last frame in the shape, and those stages draw nothing. This happens by default when `Start=` picks a later frame: the omitted `End` is the shape's whole frame count, so the animation shows the frames from `Start` to the end of the shape and then a run of blank stages. Set `End` to the number of frames the animation should show.

A [`Translucent=yes`](/keys/translucent/#scope-animtype) animation fades in steps placed at fractions of `End`, so changing `End` moves its fade steps.

## Where the stored count comes from

An animation type measures the frame count of the first shape it finds and keeps it, which is why most animations leave `End` unset. When a shape named after the animation's ID exists, the type measures that shape first. A [`DemandLoad=yes`](/keys/demandload/#scope-animtype) type that finds none measures the shape it uses in the scenario's theater when the scenario starts or a saved game is loaded. A later [`Image=`](/keys/image/#scope-animtype) that selects a different shape does not change the count, so set `End` explicitly on such an animation. A type that never finds a shape has a count of zero, so its last pass ends at its first frame advance.

`End=-1` measures the count again. The frame count of the shape the type currently uses is taken when the next animation of the type is created, and then kept for good.

:::danger[Use `End=-1` only on an animation with artwork]
An `End=-1` animation whose shape cannot be found crashes the game as soon as anything creates an animation of that type. The shape can be missing because the file does not exist, or because the theater variant of its name does not exist.
:::
