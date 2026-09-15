---
key: CD
scope: campaign
label: Campaign disc
see_also: ["Scenario", "FinalMovie", "RequiredAddon"]
when_omitted:
  kind: value
  value: "-1"
---

`0` names the GDI disc, `1` the Nod disc and `2` the Firestorm disc, while `-1` names no disc in particular. No disc is ever asked for, so the number survives only as the side the campaign is treated as belonging to.

Two decisions read it. The opening cinematic plays ahead of the first mission's briefing only while the value is below `2`. The same value also picks which film plays, and [campaign progression](/systems/campaign-progression/#the-campaign-level-number) records the file names and the fallback. The loading screen picks its backdrop from a pair of GDI pictures for `0` and a pair of Nod pictures for `1`. Each pair is two variants of the same side's loading screen at the current display size, and one of the two is drawn at random on each load. Anything above `1` falls back to searching the campaign's opening scenario file name for `GDI`, and takes the GDI pair when the name is found and the Nod pair when it is not. A negative number, `-1` included, is shown the GDI pair at every screen size.
