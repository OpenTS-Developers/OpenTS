---
key: VShift
scope: sounds
label: Loudness variation
see_also: [FShift, Volume]
when_omitted:
  kind: value
  value: "0"
---

One or two percentages of loudness change, drawn once per play. A single value quietens the play by anything up to that much, so `VShift=10` plays between ninety and one hundred percent of the sound's loudness, as Yuri's Revenge reads it. Two values are a signed range, so `VShift=-20 -5` quietens by between five and twenty percent and `VShift=0 10` may play louder than [`Volume=`](/keys/volume/) alone, up to full loudness.

```ini title="sound01.ini"
[GUN5]
VShift=15
```

The draw is kept for the whole play, so a placed sound that is re-aimed as the view scrolls does not change its loudness at random.
