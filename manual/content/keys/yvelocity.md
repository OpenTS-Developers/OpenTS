---
key: YVelocity
summary: The other horizontal axis of the spread of speeds a spark is thrown at.
see_also: ["XVelocity", "MinZVelocity", "ZVelocityRange", "BehavesLike"]
when_omitted:
  kind: value
  value: "1"
---

The second horizontal axis, and the counterpart of [`XVelocity`](/keys/xvelocity/). That page covers how the three axis settings together fix a spark's speed and direction, what the holding system adds to them, and which behaviors read them. Setting the two horizontal axes apart makes a burst spread further along one of them than the other; the stock spark types keep them equal.

:::danger[Zero stops the game]
The random speed is drawn by dividing by this figure, so `YVelocity=0` divides by zero and the game stops the moment a spark system holding the type fires its first burst.
:::
