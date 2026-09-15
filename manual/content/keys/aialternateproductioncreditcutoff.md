---
key: AIAlternateProductionCreditCutoff
summary: Credit total below which a computer house builds structures and units in alternation instead of together.
see_also: ["system:ai-base-building", "system:production"]
when_omitted:
  kind: value
  value: "1000"
---

The figure is compared against the house's loose credits plus the value of the Tiberium it holds in storage, and the comparison is made again every time one of the house's factories delivers something.

At or above the figure the house orders structures, vehicles, infantry and aircraft in the same pass. Below it the house orders one group per pass, either structures or vehicles, infantry and aircraft. A structure delivery puts it on the unit group, and a vehicle, infantry or aircraft delivery puts it on the structure group. A house already on the unit group returns to structures on a vehicle, infantry or aircraft delivery only under **Any of:**

- it owns none of the [`BuildWeapons`](/keys/buildweapons/) types;
- it owns none of the [`BuildBarracks`](/keys/buildbarracks/) types;
- it is [drawing more power than it makes](/systems/power/);
- it wins a one-in-two draw.

The narrowed pass is not a hard block. A house restricted to structures falls through to vehicles, infantry and aircraft as soon as no owned factory can produce the structure it wants. A house restricted to vehicles, infantry and aircraft falls through to structures once it has nothing pending at all, or once it has an item pending that it cannot build. While the vehicle it has pending is one of the [`HarvesterUnit`](/keys/harvesterunit/) types, that restricted house skips infantry and aircraft entirely. [Choosing what to build next](/systems/ai-base-building/#choosing-what-to-build-next) covers how the structure half of the pass picks its target.

:::note[Campaign games never narrow the pass]
The mode is only ever moved in a skirmish or multiplayer game, and the pass ignores it outright in a campaign. A computer house in a campaign orders every class together whatever its credits, so the figure decides nothing there.
:::
