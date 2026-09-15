---
key: RunningFrames
summary: The number of frames each of the eight facings of a burning victim's run cycle occupies.
see_also: ["IsFlamingGuy", "Start", "Rate"]
when_omitted:
  kind: value
  value: "0"
---

Only an [`IsFlamingGuy=yes`](/keys/isflamingguy/) animation reads the figure. A man who has been set on fire runs toward the nearest water he can see. The artwork for that run is laid out as eight consecutive blocks of this many frames, one block per facing. The block is picked from the direction he is traveling. The frame within it advances once every three game frames and wraps round, so the flames appear to face the way he goes.

Once he arrives, runs out of anywhere to go, or exhausts the number of cells he is allowed, he switches to his death sequence. That sequence begins at eight times this figure plus one and stops one frame short of the halfway point of the shape. The second half of the shape holds the shadow frames, which are drawn darkened beneath him.

The run blocks are laid out from the animation's [`Start`](/keys/start/) frame and are not checked against the artwork. A figure larger than the shape can support therefore leaves the man running on frames that draw nothing.

:::danger[A burning victim with no run cycle stops the game if it can move]
The frame within a block is worked out as a remainder taken against this figure, and the figure is not tested for zero first. An `IsFlamingGuy=yes` animation that leaves it unstated, or states `RunningFrames=0`, divides by zero and faults on the first logic pass in which it has somewhere to run to. One created with nowhere left to go switches straight to its death sequence and never reaches the division.
:::
