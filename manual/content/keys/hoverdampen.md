---
key: HoverDampen
summary: Fraction of its vertical motion a hovering unit keeps from one frame to the next.
see_also: [HoverHeight, HoverBob, Gravity]
when_omitted:
  kind: value
  value: ".8"
---

Every frame the cushion adds its lift, subtracts [`Gravity`](/keys/gravity/), and then multiplies what is left over by this fraction. Subtracting `Gravity` is a constant pull rather than a decay, so this fraction is the only thing that bleeds a bounce away. Below `1`, a hover unit shoved down or crossing a rise settles back to [its rest height](/keys/hoverheight/) instead of oscillating about it. The stock rules write it as `40%`, which the engine reads as `0.4`; a plain `0.4` is the same assignment. Only the percent sign divides by a hundred, so `40` on its own is forty. That shipped line is what a stock game stores, and the fallback above stands only for a rules file that leaves the assignment out. The levitation locomotor damps on the same fraction.

:::caution[At 1 and above the bounce never settles]
A fraction of `1` keeps the whole of the accumulated vertical motion. The unit then swings about its rest height for as long as it exists. Above `1` the swing grows every frame, though being driven into the ground clears the accumulated motion and starts it over.
:::
