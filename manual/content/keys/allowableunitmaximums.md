---
key: AllowableUnitMaximums
summary: How many of each restricted type a mission's dropship loadout may hold.
see_also: [AllowableUnits, StartingDropships]
when_omitted:
  kind: computed
  note: "Roughly half the missing numbers: the padding loop re-reads both list lengths on every turn, so it closes only half the gap between the two, and the pairing lookup then reads past the end of the shorter list for every name beyond it."
---

```ini title="map file"
[Basic]
AllowableUnits=E1,E2,SMECH
AllowableUnitMaximums=-1,-1,2
```

Each number pairs by position with the name at the same place in [`AllowableUnits`](/keys/allowableunits/) and caps how many of that type may be loaded across all of the mission's dropships together. Three values behave distinctly:

| Value | Effect |
| --- | --- |
| `-1` | The type is offered and never counted, so it may fill the whole loadout |
| `0` | The type is dropped from the cameo list and cannot be loaded at all |
| Positive | The type is offered until that many have been taken aboard, then its cameo stops responding |

A list shorter than its companion is padded with `-1`, but the padding closes only half the gap. The loop re-counts the number list after every entry it adds, so padding stops once the number list holds about half as many entries as the name list. Three names and no numbers gain two numbers, leaving one name unpaired. An unpaired name is still looked up at its own position, and that position in the number list was never written; the value that answers is whatever the list's storage holds past its last entry. A longer list keeps its extra numbers, and they pair with nothing and do nothing.

:::caution[A mismatched pair silently shifts every later limit]
Nothing checks that the two lists describe the same units. Writing them in a different order, or dropping a name and leaving its number in place, applies each remaining limit to whichever type now sits at that position.
:::
