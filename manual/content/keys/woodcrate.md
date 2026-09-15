---
key: WoodCrate
summary: The crate result delivered by a WoodCrateImg crate in a campaign.
see_also: ["system:crates"]
when_omitted:
  kind: value
  value: Money
---

Campaign crates are not drawn at random: the overlay determines the result outright. This setting is the result for a crate whose overlay is the one named by [`WoodCrateImg`](/keys/woodcrateimg/). The value is one of the [crate result](/reference/enums/crate/) tokens. A value the engine does not recognize resolves to `Money` with no diagnostic, so a misspelled token is indistinguishable from writing `Money`.

The campaign lookup runs this test last. A rules file that names the same OverlayType in `WoodCrateImg` and [`CrateImg`](/keys/crateimg/) therefore makes this setting the result for every campaign crate.
