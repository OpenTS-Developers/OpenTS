---
key: AllowableUnits
summary: The infantry and vehicle types offered on a mission's dropship loadout screen.
see_also: [AllowableUnitMaximums, StartingDropships, TechLevel]
when_omitted:
  kind: value
  value: ""
  note: An empty list is not a ban; it hands the loadout screen its own selection of everything the player's house could field.
---

```ini title="map file"
[Basic]
StartingDropships=2
AllowableUnits=E1,E2,SMECH
AllowableUnitMaximums=-1,-1,2
```

The list decides which cameos the dropship loadout screen shows before a mission that opens with [`StartingDropships`](/keys/startingdropships/) above zero. It is read nowhere else, so a mission without that screen is unaffected by it.

With the list empty, the screen builds its own selection. It considers every infantry and vehicle type, and keeps a type only under **None of:**

- its build level is above the house's tech level;
- it is civilian infantry;
- its build level is `-1`;
- it costs `10` or less;
- the player's house may not own it.

With the list populated, that whole filter is replaced: exactly the types named are offered, subject only to house ownership and to a maximum of `0` hiding an entry. Build level and cost no longer matter, so a mission can offer a unit the player could not otherwise build.

Entries pair by position with [`AllowableUnitMaximums`](/keys/allowableunitmaximums/), so both lists have to be written in the same order.

:::caution[An unrecognized ID is dropped rather than reported]
A name matching no registered type at all is skipped as the list is parsed, and the surviving entries close the gap. Every maximum written after the missing name then applies to the wrong type.
:::
