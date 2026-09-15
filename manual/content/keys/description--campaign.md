---
key: Description
scope: campaign
label: Campaign list entry
see_also: ["Scenario", "RequiredAddon", "FinalMovie"]
when_omitted:
  kind: computed
  note: The campaign's INI name, which is the `[Battles]` value and its section name, stands in as the row text.
---

The mission selection list is assembled from the `[Battles]` section of every `BATTLE*.INI` file [the file search](/formats/opents-ini/#the-order-files-are-searched-for-in) covers. The files are read in alphabetical order. The expansion's battle file is then parsed a second time, after every other one. The re-read finds each campaign it named on the first pass already present, updates it in place, and leaves the list with no duplicate rows. That second pass is also the last read those campaigns get, so where an alphabetically later battle file sets a different value for one of them, the expansion's value is the one that keeps. The rows keep the order the campaigns are first named. The text is the row the campaign occupies in that list, so it is the only part of a campaign the player sees before choosing it. Campaigns the running expansion does not allow are left out of that list, so their text is never displayed.

Each `[Battles]` entry's value names one campaign, and that name is the section the campaign's own assignments, this key included, are read from. The number to the left of the `=` is never read: the section is walked by position and only each entry's value is taken.

```ini title="BATTLE.INI"
[Battles]
1=GDI1

[GDI1]
Description=REUNION
Scenario=Maps/Missions/GDI1A.MAP
CD=0
```

At most 127 characters are kept; a longer line is cut.
