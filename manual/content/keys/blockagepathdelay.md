---
key: BlockagePathDelay
summary: Frames a ground object spends preferring to wait out a moving obstruction before it insists on routing around one.
see_also: [PathDelay]
when_omitted:
  kind: value
  value: "60"
---

The value is a raw frame count, so the default is four seconds at 15 frames to the second. For a two-second wait the line reads:

```ini title="rules.ini"
[AI]
BlockagePathDelay=30
```

It is loaded into the object's own countdown twice: once when the object is given a destination, and again the first time a step is refused because another moving object stands in the way. Each load sets the full figure again rather than adding to the time left.

While that countdown is running, a retry prices a cell held by a moving object at four times a clear step. That usually finds the same route again, which leaves the object waiting for the other one to clear. Once the countdown reaches zero, the same cell is priced at a thousand times a clear step instead. The search then takes almost any detour over waiting, but it still routes through the blocker when no detour exists. Raising the figure makes traffic queue up behind itself for longer; lowering it sends objects around each other sooner and spreads them over more ground.

The driving, walking, mechanical and hovering locomotors all use the same countdown. Reaching zero only changes what a retry asks for: the object is not rerouted until it is blocked again.
