---
key: TranslucencyDetailLevel
summary: The lowest detail setting at which the animation is drawn faded rather than solid.
see_also: ["Translucency", "Translucent", "DetailLevel"]
when_omitted:
  kind: value
  value: "0"
---

Blending an animation into the scene costs more than drawing it solid, and this figure is where that cost is given up. While the player's own detail setting has reached it, the animation's fade is worked out and applied. Below it, no fade is worked out at all and the animation is drawn solid.

The player's setting runs from 0 at its lowest through 2 at its highest. `1` drops the fade at the lowest setting, `2` keeps it only at the highest, and anything above 2 drops it at every setting. At zero the fade is applied whatever the player has chosen.

The figure gates every route to a fade, not just the type's own:

- the by-stage ramp of a [`Translucent=yes`](/keys/translucent/#scope-animtype) animation;
- the fixed figure of [`Translucency=`](/keys/translucency/#scope-animtype);
- the fade a structure passes down to the animations it is running as it goes out of sight.

```ini title="art.ini"
[MYRING1] ; a blast ring, drawn solid on the lowest detail setting
Image=MYRING1
Translucent=yes
TranslucencyDetailLevel=1
Rate=300
Flat=true
```

:::caution[Raising this can leave a hidden structure's animations on screen]
The decision that is skipped below the threshold includes the test that stops an animation being drawn once it has faded out completely. A structure that cloaks or otherwise fades from sight takes its animations down with it only while the fade is being worked out. Below the threshold that test is never reached, so those animations keep being drawn at full opacity over a structure that is no longer there. Giving [`DetailLevel`](/keys/detaillevel/#scope-animtype) the same figure avoids it, since the animation is then dropped altogether at the settings where its fade would be.
:::

This figure decides only how the animation is drawn. Whether it is drawn at all is the separate threshold in [`DetailLevel`](/keys/detaillevel/#scope-animtype).
