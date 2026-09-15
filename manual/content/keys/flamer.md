---
key: Flamer
summary: Throws one to three fires onto the ground around the animation.
see_also: ["Scorch", "SmallFire", "LargeFire"]
when_omitted:
  kind: value
  value: "no"
---

On the frame whose artwork covers the most ground, the animation creates the rules' [`SmallFire`](/keys/smallfire/) at the nearest free spot 64 leptons away in a random direction. It then has an even chance of a second `SmallFire` 160 leptons out and an even chance of a [`LargeFire`](/keys/largefire/) 112 leptons out, each direction picked separately. Every fire runs for one or two times the loop count the fire type sets. A looping animation throws a fresh batch each time it comes back to that frame.

Nothing filters the fires by height or by the ground underneath, so a flame thrower animation playing over water or high in the air still scatters them.

The setting displaces the single fire that [`Scorch=yes`](/keys/scorch/) would otherwise leave, and does not itself scorch or crater anything. An animation that should blacken the ground as well needs `Scorch=yes` alongside it.

:::note[No stock animation declares it]
The shipped `art.ini` sets the flag on nothing, so the behavior is reachable only from an animation a mod flags itself.
:::
