---
key: CrateImg
summary: The OverlayType whose crates deliver the SilverCrate result in a campaign.
see_also: ["system:crates"]
when_omitted:
  kind: value
  value: none
---

The engine never places this overlay. It reaches a map only when the map author draws it into the overlay layer. That also means it appears only in a campaign: crates in a map's overlay layer are discarded in every other game type. Collecting it selects [`SilverCrate`](/keys/silvercrate/).

:::caution[The two image settings must name different overlays]
The campaign lookup tests this setting first and [`WoodCrateImg`](/keys/woodcrateimg/) second, and the second test overwrites the first. Naming one OverlayType in both settings therefore turns every crate into a [`WoodCrate`](/keys/woodcrate/) and leaves `SilverCrate` unreachable. The shipped rules do this, and the silver crate needs an OverlayType of its own with [`Crate=yes`](/keys/crate/) before it can be used.
:::
