---
title: Turn siliconsun behavior off from SUN.INI
category: feature
release: 0.2.0
targets:
- type: key
  id: DoubleClickSelectType
  effect: added
- type: key
  id: CampaignIntroMovie
  effect: added
credit:
- fjlaubscher
---

`DoubleClickSelectType=no` under `[siliconsun]` in `SUN.INI` leaves a double-click as an ordinary click. `yes` selects every on-screen unit of the double-clicked unit's type.

`CampaignIntroMovie=no` under the same section leaves the movie archives in alphabetical order. `yes` plays a campaign introduction from that campaign's movie archive.
