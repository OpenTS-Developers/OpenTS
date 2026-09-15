---
key: Length
summary: The playing time of the score, in minutes, shown beside its name on the sound options screen.
see_also: [Name, Normal]
when_omitted:
  kind: value
  value: "0"
---

The figure is in minutes and may be fractional. It is multiplied by 60 into whole seconds and printed as minutes and seconds beside the score's [`Name`](/keys/name/#scope-themes) in the sound options track list. That is the only thing the figure does. Nothing times the playback from it, so a figure that disagrees with the audio file misreports the track's length without cutting it short or letting it overrun.

```ini title="theme.ini"
[VALVES1B]
Name=Valves
Length=3.27
```

`3.27` is three minutes and 16 seconds, not three minutes and 27 seconds. The fraction is a fraction of a minute rather than a count of seconds.
