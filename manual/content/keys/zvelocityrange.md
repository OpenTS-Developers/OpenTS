---
key: ZVelocityRange
summary: The spread of upward speeds a spark is thrown at.
see_also: ["MinZVelocity", "XVelocity", "YVelocity", "BehavesLike"]
when_omitted:
  kind: value
  value: "1"
---

How far above [`MinZVelocity`](/keys/minzvelocity/) the initial upward speed of a particle thrown by a spark system may reach, in leptons a frame. [`XVelocity`](/keys/xvelocity/) says which systems read the axis settings. Each spark draws a random amount below this figure and adds it to that floor. The range alone decides how ragged the top of a shower looks; the floor decides how high it goes. The stock spark types set `MinZVelocity` to `40` and this figure to `15`, so every spark starts out 40 to 54 leptons a frame upward.

The one random direction a whole burst shares draws its vertical part from this figure on its own, without the floor. That draw may be negative, so a burst thrown in a random direction can be tipped downward, even though no individual spark's own draw ever is.

:::danger[Zero stops the game]
The random amount is drawn by dividing by this figure, so `ZVelocityRange=0` divides by zero and the game stops the moment a spark system holding the type fires its first burst.
:::
