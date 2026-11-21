---
key: Category
summary: Classifies an object, which the dropship loadout screen reads to keep civilians off its purchase list.
see_also: [Points, ThreatPosed]
when_omitted:
  kind: value
  value: none
---

The value names one of the [object categories](/reference/enums/object-category/), in either its short or long spelling and in any case. A value that matches neither spelling leaves the type with no category, discarding any category set earlier.

The category has one effect. An InfantryType with `Category=Civilian` is left off the list of types a player can buy on the dropship loadout screen. That screen opens at the start of a campaign mission whose `[Basic] StartingDropships` is above zero. The civilian is left off only when the mission lists no [`AllowableUnits`](/keys/allowableunits/); a mission that lists types can offer a listed civilian like any other listed type.
