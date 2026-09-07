---
key: BuildConst
summary: The construction yard BuildingTypes; a building of any listed type is a construction yard.
see_also: ["system:ai-base-building"]
when_omitted:
  kind: value
  value: ""
---

A building of any listed type counts towards its house's construction yards and, as a factory, produces only for the country its own record says it acts as, so a house whose yard is a later entry builds as one whose yard is entry 0 does; [`ConstructionYard=yes`](/keys/constructionyard/) on a type not listed here does not stand in for it. While [the base plan](/systems/ai-base-building/#building-the-plan) is assembled, a [`Prerequisite`](/keys/prerequisite/) naming any entry counts as already met, and the plan is seeded with the first entry the house may own. A UnitType whose [`DeploysInto`](/keys/deploysinto/) names any entry is an MCV a base-building computer house acts on: outside a campaign game it sends one out to hunt while it owns no construction yard, and it deploys one standing guard instead of leaving it there.

:::caution[Building lists are split on commas alone]
Names are matched without regard to case, and while the value is trimmed at its ends, the split is on commas alone, so `GAPOWR, NAPOWR` looks for a type whose ID begins with a space. A name matching no BuildingType ID — an existing UnitType or InfantryType ID included — adds a new BuildingType carrying nothing but that name, which no house may own, instead of reporting a problem. Every building list in `[AI]` is read like this, and a scenario carrying its own `[AI]` section replaces each list it names.
:::
