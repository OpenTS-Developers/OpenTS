---
key: DetailLevel
scope: animtype
label: Animation detail threshold
see_also: ["TranslucencyDetailLevel", "Translucency"]
when_omitted:
  kind: value
  value: "0"
---

The animation is drawn only while the player's own detail setting has reached this figure. That setting runs from 0 at its lowest through 2 at its highest. A figure of `1` drops the animation at the lowest setting, `2` keeps it only at the highest, and anything above 2 hides it at every setting. At zero the test never hides anything. A purely decorative animation sets a high figure so that it is the first thing to go on a slow machine.

The test governs drawing alone. An animation held back by it is still created, still advances through its stages, still counts its passes down and still applies whatever damage, craters, scorch marks or tiberium growth it has. Only the artwork is skipped. Raising the figure therefore trims drawing work without changing what happens in the game.

The stand-in a structure's animation leaves behind under the fog of war makes the same test, so an animation the setting hides is missing from the remembered picture as well as from the live one.

How faded the animation is drawn is gated separately, by its own [`TranslucencyDetailLevel`](/keys/translucencydetaillevel/); this figure decides only whether it is drawn at all.

```ini title="art.ini"
[MYPILE_A] ; an idle animation for a barracks
Image=MYPILE_A
LoopStart=0
LoopEnd=8
LoopCount=-1
Rate=300
Surface=yes
DetailLevel=1 ; not drawn at the lowest detail setting
```
