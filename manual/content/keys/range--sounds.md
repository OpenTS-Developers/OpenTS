---
key: Range
scope: sounds
label: Hearing distance
see_also: [Type, MinVolume, Volume]
when_omitted:
  kind: value
  value: "28"
---

How far from the view a sound played at a place in the world is still heard, in cells of forty-eight pixels. The loudness falls in a straight line from full at the edge of the view to silence at this distance beyond it, with vertical distance counting double because the view is wider than it is tall; below five percent the sound is not started at all. With `LOCAL` in the sound's [`Type=`](/keys/type/) the distance runs from the centre of the view, and with `GLOBAL` the fall stops at [`MinVolume=`](/keys/minvolume/).

```ini title="sound01.ini"
[GUN5]
Range=20
```

The default of 28 cells is 1344 pixels, within one percent of the 1360 the original game faded over, so the shipped sounds are heard from where they were.
