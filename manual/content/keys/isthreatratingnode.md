---
key: IsThreatRatingNode
summary: Marks a building upgrade that switches its owner's house onto the per-type threat coefficients.
see_also: ["system:target-selection"]
when_omitted:
  kind: value
  value: "no"
---

Two events set the house flag, and both are on the upgrade path: plugging a building with this setting into a host as an upgrade, and removing a host that already has such an upgrade plugged into it. A standalone building with the setting (one that is not a [`PowersUpBuilding`](/keys/powersupbuilding/) upgrade and so is never plugged into anything) sets the flag on neither event.

:::caution[Every house already has the flag]
A house sets it as it is created and nothing ever clears it, so the per-type threat coefficients are in force for every house from the first frame of the game. This setting can therefore only re-set a flag that is already set, and the house-wide coefficients it was meant to switch away from are never reached.
:::
