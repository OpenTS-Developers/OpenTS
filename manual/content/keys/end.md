---
key: End
summary: The number of stages the animation runs through before it ends or chains.
see_also: ["Start", "LoopEnd", "LoopCount", "Next", "Rate"]
when_omitted:
  kind: computed
  note: The frame count of the shape the animation draws, or zero when it has none.
---

The animation shows one frame of its shape per stage, starting at [`Start`](/keys/start/). Stages count from zero, so the frames shown are `Start` through `Start` + `End` - 1. When the stage counter reaches `End`, the animation is removed or becomes its [`Next=`](/keys/next/) animation in the same logic pass, and that stage is never drawn.

`End` limits only the last pass. While an animation still has passes left to run, each pass ends at [`LoopEnd`](/keys/loopend/) instead. Without [`LoopCount=`](/keys/loopcount/), the animation makes one pass.

`End` can run past the last frame in the shape, and those stages draw nothing. This happens by default when `Start=` picks a later frame: the omitted `End` is the shape's whole frame count, so the animation shows the frames from `Start` to the end of the shape and then a run of blank stages. Set `End` to the number of frames the animation should show.

A [`Translucent=yes`](/keys/translucent/#scope-animtype) animation fades in steps placed at fractions of `End`, so changing `End` moves its fade steps.

## Counts taken from the shape

An omitted `End`, `End=0` and `End=-1` all mean the frame count of the shape the animation draws, which is why most animations leave `End` unset. The count is read from the shape whenever the animation needs it, including while it plays, so it always matches the shape the type has at that moment: the [`Image=`](/keys/image/#scope-animtype) shape, the variant for the current scenario's theater, or a [`DemandLoad=yes`](/keys/demandload/#scope-animtype) shape loaded when the scenario started or a saved game was loaded.

An animation with no shape draws nothing. Unless `End` is set to a positive number, it counts zero stages, and its last pass ends at its first frame advance.
