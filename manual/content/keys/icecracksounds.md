---
key: IceCrackSounds
summary: The sounds a snow-theater ice tile picks from as it cracks.
see_also: [IceCrackingWeight, IceBreakingWeight, IceSolidifyFrameTime, Weight]
when_omitted:
  kind: value
  value: ""
  note: The list is emptied before the read, so a file that declares the section without this key clears whatever an earlier file set rather than keeping it.
---

```ini title="rules.ini"
[AudioVisual]
IceCrackSounds=ICECRK1,ICECRK2 ; sound IDs registered in SOUND.INI
```

One entry is picked at random and played once at full volume, on the pass that turns an intact ice tile into its cracked form. Only the snow theater has ice. Two things crack it: a vehicle whose [`Weight`](/keys/weight/) reaches [`IceCrackingWeight`](/keys/icecrackingweight/) but stays under [`IceBreakingWeight`](/keys/icebreakingweight/) entering the cell, and a warhead with [`Wall=yes`](/keys/wall/#scope-warheadtype) or [`Fire=yes`](/keys/fire/) detonating on it. A vehicle riding a bridge over the top cracks nothing.

Crossing an already-cracked tile drops the ice out from under the crosser instead, and that has no sound of its own. Weight enough to break the ice outright skips the cracked stage and is equally silent.

The list is checked for length before an entry is picked, so leaving it empty is silence rather than a crash. Names are matched against the IDs registered in [SOUND.INI](/formats/sound-ini/) and anything unmatched is dropped, so a list of nothing but misspellings ends up empty.

:::caution[Declaring the section elsewhere wipes the list]
This is the one entry in `[AudioVisual]` that is cleared before it is read rather than falling back on its current value. Any later file with an `[AudioVisual]` section (a language rules file, the Firestorm rules, or a map) empties the list unless it names the key again. Every other setting in the section survives such a file untouched.
:::
