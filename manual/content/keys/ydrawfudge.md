---
key: YDrawFudge
summary: Pixels a terrain object's artwork is shifted down the screen by.
when_omitted:
  kind: value
  value: "0"
---

The figure is added to the vertical screen position the object draws at, so a positive figure moves the artwork down the screen and a negative one lifts it. It is applied to the position of the ordinary draw, and to the render rectangle worked out once when the object is placed. The tactical map dirties that rectangle when the object needs refreshing.

```ini title="rules.ini"
[MYROCK]         ; example boulder whose artwork sits high in its frame
YDrawFudge=-6    ; lift it six pixels so it meets the ground
```

The ordinary draw also biases the object's depth by a third of the figure, discarding the remainder, so any figure from `-2` to `2` biases the depth by nothing at all. That bias is what decides how a placed object sorts against its neighbors. The whole figure goes into the render rectangle, which affects only how much of the map gets redrawn.
