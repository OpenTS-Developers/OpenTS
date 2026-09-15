---
key: Smoke
summary: Parsed animation that the engine never creates.
no_effect: true
see_also: [SmallFire, LargeFire, DropPodPuff]
when_omitted:
  kind: value
  value: none
---

The name promises the smoke that rises from a wrecked structure. The one assignment is read twice, into two separate slots, so both receive the same animation. No gameplay path reads either slot. Only the save-game pointer fixup and the type-removal sweep touch them.

The value is still resolved as it is read, and a name the rules do not already list is registered as an animation on the spot. The shipped rules assign `xxxx` here rather than any real animation, which costs nothing only because both slots are dead.
