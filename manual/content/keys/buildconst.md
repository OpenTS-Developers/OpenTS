---
key: BuildConst
summary: The construction yard BuildingTypes; a building of any listed type is a construction yard.
see_also: ["system:ai-base-building"]
when_omitted:
  kind: value
  value: ""
---

A building of any listed type counts towards its house's construction yards. As a factory it produces only for the country its own record says it [acts as](/keys/actslike/), so a house whose yard is a later entry builds as one whose yard is entry 0 does. [`ConstructionYard=yes`](/keys/constructionyard/) on a type not listed here does not stand in for it. While [the base plan](/systems/ai-base-building/#building-the-plan) is assembled, a [`Prerequisite`](/keys/prerequisite/) naming any entry counts as already met, and the plan is seeded with the first of the listed types that the plan's candidate scan accepts. A UnitType whose [`DeploysInto`](/keys/deploysinto/) names any entry is an MCV a base-building computer house acts on. Outside a campaign game it sends one out to hunt while it owns no construction yard. An MCV standing guard is ordered to deploy instead of staying put.

:::caution[Building lists are split on commas alone]
Names are matched without regard to case, and while the value is trimmed at its ends, the split is on commas alone, so `GAPOWR, NAPOWR` looks for a type whose ID begins with a space. A name matching no BuildingType ID (an existing UnitType or InfantryType ID included) adds a new BuildingType with nothing but that name, which no house may own, instead of reporting a problem. Every building list in `[AI]` is read like this, and a scenario with its own `[AI]` section replaces each list it names.
:::
