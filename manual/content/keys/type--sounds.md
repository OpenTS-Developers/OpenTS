---
key: Type
scope: sounds
label: Where a sound is heard from
see_also: [Range, MinVolume, Control]
when_omitted:
  kind: value
  value: SCREEN
---

Flags, separated by spaces or commas, saying how a sound played at a place in the world is heard. The values follow Yuri's Revenge.

- `NORMAL` or `SCREEN`: the sound fades with the distance of its place from the edge of the view, vertical distance counting double, reaching silence at [`Range=`](/keys/range/), and is panned by where that place is across the view.
- `LOCAL`: as `SCREEN`, but the distance is measured from the centre of the view, so a sound at the edge is already quieter.
- `GLOBAL`: the fade stops at [`MinVolume=`](/keys/minvolume/) instead of silence.
- `SHROUD` or `UNSHROUDED`: silent unless the cell of its place has been revealed.
- `SHROUDED`: silent unless the cell of its place is still unrevealed.
- `UNSHROUD`, `VIOLENT`, `MOVEMENT`, `QUIET`, `LOUD`, `PLAYER`, `NOISE_SHY`, `GUN_SHY` and `AMBIENT` are read and kept but decide nothing, as they did not in Yuri's Revenge.

```ini title="sound01.ini"
[BIGBLAST]
Type=GLOBAL SHROUD
MinVolume=0.3
```

A sound played without a place, such as a button click, is not attenuated or panned at all, whatever its type. A flag the engine does not know is ignored.
