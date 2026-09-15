---
key: TeamDelays
summary: The frames a house waits between team creation passes, one entry per difficulty.
see_also: ["system:ai-team-production"]
when_omitted:
  kind: value
  value: ""
  note: The list stays empty and the difficulty slot is used to index it anyway, reading storage that was never allocated.
---

Entries run hardest game setting first: a computer house takes the first entry when the player chose the hardest setting and the last when they chose the easiest. The index is the house's own [difficulty slot](/systems/difficulty/#from-the-setting-to-a-slot). The stock rules set `2250,2700,3600`, or two and a half to four minutes at 900 frames to the minute. Give one entry per difficulty; a shorter list is indexed the same way for the slots it does not cover.

A house's countdown starts at this value plus `175` frames for each place it holds in the house list, so houses do not all run their [team creation pass](/systems/ai-team-production/#when-the-pass-runs) on the same frame. The reset after a pass drops the stagger and uses this value alone. A `0` leaves the countdown at zero, so the house runs a pass every frame.
