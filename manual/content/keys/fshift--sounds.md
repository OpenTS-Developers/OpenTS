---
key: FShift
scope: sounds
label: Pitch variation
see_also: [VShift]
when_omitted:
  kind: value
  value: "0"
---

One or two percentages of pitch shift. A play draws one value between the two and plays every sample of that play that much higher or lower; a single value spans both ways, so `FShift=5` is the same as `FShift=-5 5`. The result is held between half and double the recorded pitch. The shift changes the speed of the sample as well as its pitch.

```ini title="sound01.ini"
[GUN5]
FShift=-8 8
```
