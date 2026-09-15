---
key: WoodCrateImg
summary: The OverlayType used for every crate the engine places itself.
see_also: ["system:crates"]
when_omitted:
  kind: value
  value: none
---

Both engine-driven placements use this overlay: the random crates scattered outside a campaign and the crate dropped by a destroyed [`CarriesCrate=yes`](/keys/carriescrate/) vehicle. It is also the second of the two overlays tested when a campaign crate is collected, where it selects [`WoodCrate`](/keys/woodcrate/).

:::danger[Random crate placement requires this setting]
The placement routine creates the overlay without checking that this setting names anything, and the new overlay immediately reads its own type back. A match that reaches random crate placement with no OverlayType named here crashes on the first attempt. Every non-campaign scenario reaches that path during setup while crates are enabled.
:::

Naming the same OverlayType here and in [`CrateImg`](/keys/crateimg/) makes [`SilverCrate`](/keys/silvercrate/) unreachable; [Choosing the result](/systems/crates/#in-a-campaign) covers why.
