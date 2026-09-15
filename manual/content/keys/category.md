---
key: Category
summary: Classifies an object, which the dropship loadout screen reads to keep civilians off its purchase list.
see_also: [Points, ThreatPosed]
when_omitted:
  kind: value
  value: none
---

The value is matched against the [object categories](/reference/enums/object-category/), which accept either the short token or the longer editor description and ignore case. A value matching neither spelling discards the stored category and leaves the type uncategorized.

An InfantryType categorized `Civilian` is left out of the cameo list on the dropship loadout screen. That screen opens when the scenario's `[Basic] StartingDropships` is above zero, and the civilian is left out only on the fallback list it builds when the scenario declares no [`AllowableUnits`](/keys/allowableunits/) of its own.

Nothing else reads the category.
