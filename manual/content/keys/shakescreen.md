---
key: ShakeScreen
summary: Divisor for a shake count that nothing acts on.
see_also: [Cost, Strength]
when_omitted:
  kind: value
  value: "400"
---

```ini title="rules.ini"
[AudioVisual]
ShakeScreen=400
```

A destroyed object divides a figure of its own by this value to get a number of shakes. A structure being torn down divides its house-adjusted [`Cost`](/keys/cost/) by it and shakes that many times if the result is at least one. A vehicle whose type names at least one explosion animation and whose [`Strength`](/keys/strength/) is above this value divides that strength by half of it, adds three, and caps the result at six. A larger value therefore means fewer objects are worth a shake.

Neither count reaches anything. The routine both hand their result to has an empty body, so no value of this setting moves the view. The only screen displacement the game still performs is the fixed ten-pixel jolt an ion cannon blast applies on the frame it lands, and that does not read this setting.

:::danger[Three values crash the game as an object is destroyed]
The divisions are integer and unguarded. At `0` a structure being destroyed divides by zero. At `0`, `1` or `-1` the vehicle path halves the value to zero and a destroyed vehicle divides by zero. Either one takes the process down at the moment the object dies, so a value in that range is a crash waiting on the first casualty rather than a setting that does nothing.
:::
