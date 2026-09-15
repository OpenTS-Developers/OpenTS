---
key: PathDelay
summary: How long a ground object waits before searching for a route again after a search that failed.
see_also: [BlockagePathDelay]
when_omitted:
  kind: value
  value: ".016"
---

The value is given in minutes and turned into a whole number of frames, so the default is 14 frames at 900 frames to the game minute. Each object has its own countdown, and every ground locomotor reads it before running a search. Wheeled and tracked vehicles, walking infantry, mechanical walkers and hovering craft all stand in place while the countdown runs, and try again on a later frame.

A search that finds a route clears the countdown at once, so the wait falls only on objects that could not be routed. Something walled in by terrain, or ordered to a place it cannot reach, retries on this interval instead of every frame. The countdown is also cleared outright when the object is given a new destination, so a fresh order is never held up by a wait left over from the last one.

Retrying around a blocking object is throttled by the same countdown, and there the wait is set whether or not the retry found anything.

:::caution[Lowering this spends search time every frame]
At `0` the countdown is never running. An object that cannot be routed then runs a full route search on every frame it is asked to move, and every such object pays that cost separately.
:::
