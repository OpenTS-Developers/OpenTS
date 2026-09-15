---
key: SilverCrate
summary: The crate result delivered by a CrateImg crate in a campaign.
see_also: ["system:crates"]
when_omitted:
  kind: value
  value: HealBase
---

Campaign crates are not drawn at random: the overlay determines the result outright. This setting is the result for a crate whose overlay is the one named by [`CrateImg`](/keys/crateimg/). The value is one of the [crate result](/reference/enums/crate/) tokens. An unrecognized token resolves to `Money` with no diagnostic.

The setting has no effect at all while `CrateImg` and [`WoodCrateImg`](/keys/woodcrateimg/) name the same OverlayType, which is how the shipped rules are written. [Choosing the result](/systems/crates/#in-a-campaign) covers the mechanism and what a second overlay needs.
