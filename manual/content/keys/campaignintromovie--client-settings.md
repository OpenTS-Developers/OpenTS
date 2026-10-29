---
key: CampaignIntroMovie
scope: client-settings
label: Campaign introduction
summary: Plays a campaign's introduction from that campaign's movie archive.
when_omitted:
  kind: value
  value: "yes"
---

`CampaignIntroMovie=yes` searches the campaign's own movie archive ahead of the other movie archives. Disc `0` prefers `MOVIES01.MIX` and disc `1` prefers `MOVIES02.MIX`. The introduction still plays only when the first mission of a campaign starts with its briefing, and only when that campaign's `CD=` is below `2`. `INTR<n>.VQA` plays when that file is present; otherwise `INTRO.VQA` plays from the first archive in the search order that contains it.

`no` leaves the movie archives in alphabetical order, so `INTRO.VQA` comes from the first of those archives. `INTR<n>.VQA` still plays for side `n` when that file is present.

The key is read from `[siliconsun]` in `SUN.INI` at startup. No dialog offers it. Leaving the options menu writes the value currently in force.
