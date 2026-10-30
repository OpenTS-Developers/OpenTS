---
key: Name
scope: themes
label: Track display name
see_also: [Length, Normal]
when_omitted:
  kind: value
  value: ""
  note: An empty name, which leaves the track's row in the track list without a title.
---

`Name=` sets the track's title as players see it. The title appears in the sound options track list, beside the track's [`Length`](/keys/length/), in the on-screen message that the [next track](/commands/nexttheme/) and [previous track](/commands/prevtheme/) commands show, and on the now-playing line while [`NowPlaying=yes`](/keys/nowplaying/). The value is shown exactly as written, not looked up as a translatable string. Only the first 63 characters are kept.

```ini title="theme.ini"
[DUSKHOUR]
Name=Dusk Hour
Length=4.11
Scenario=1
Side=GDI
```

The section name is the track's ID. It names the track's file unless [`Sound=`](/keys/sound/) names another, and it is how other settings refer to the track. A value that refers to a track is matched in two steps, so it can also select a track by its display name:

1. The value is compared with every track ID, ignoring case. A match selects that track.
2. If no ID matches, the first track in `[Themes]` order whose display name contains the value, also ignoring case, is selected.

Every value that refers to a track is matched this way. That covers a scenario's [`Theme=`](/keys/theme/), the tracks that the main menu, map selection, the score screen and ion storms request by ID, and the IDs listed under `[Themes]`. With the example above, and no track whose ID is `Dusk`, `Theme=Dusk` and `Theme=DUSK` both select `DUSKHOUR`.

A `[Themes]` entry can therefore fail to add a track. If its ID matches no earlier track's ID but appears inside an earlier track's display name, the entry is taken as that earlier track. No new track is added, and the entry's own section is never read.
