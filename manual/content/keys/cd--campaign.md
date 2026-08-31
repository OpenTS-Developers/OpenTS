---
key: CD
scope: campaign
label: Campaign disc
see_also: ["Scenario", "FinalMovie", "RequiredAddon"]
when_omitted:
  kind: value
  value: "-1"
---

`0` names the GDI disc, `1` the Nod disc and `2` the Firestorm disc, while `-1` names no disc. The value selects the campaign side and identifies the disc that supplies its files; OpenTS does not prompt for that disc.

For values below `2`, OpenTS plays an opening cinematic before the first mission's briefing. It reads `INTRO.VQA` from `MOVIES<nn>.MIX`, counting from `01`, because the base-game discs carry different films under that name. Earlier override archives keep their priority, but the [other movie archives](/formats/mix/#mounting-and-search-order) are omitted from this lookup. If the selected archive is not mounted, the normal search order applies.

The same value selects the loading-screen backdrop. `0` uses one of two GDI pictures and `1` uses one of two Nod pictures. For values above `1`, OpenTS searches the campaign's opening scenario filename for `GDI`; a match uses the GDI pictures and no match uses the Nod pictures.

:::caution[A campaign that names no disc reads its backdrop name from outside the table]
The backdrop table is indexed straight from the disc number, so `-1` lands two places in front of it. The offsets used at 640 by 480, and at 800 by 600 or larger, carry the index back inside the table, but at every other screen size the file name is taken from the bytes in front of it.
:::
