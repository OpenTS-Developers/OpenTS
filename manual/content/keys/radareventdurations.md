---
key: RadarEventDurations
summary: The frames a radar event stays in play, one entry per event kind.
see_also: ["system:map-visibility", RadarEventVisibilityDurations, RadarEventSuppressionDistances]
when_omitted:
  kind: value
  value: ""
  note: The list stays empty and an event's own kind is used to index it anyway, dereferencing a null pointer.
---

Entries are positional: one per [radar event](/reference/enums/radar-event/) kind, in the order that page lists them. An event indexes the list with its own kind. The timer is loaded on the frame the box stops turning rather than when the event is raised, so the sweep inward costs the event nothing. Entries are game frames, 900 to the game minute.

```ini title="rules.ini"
[General]
; combat, non-combat, drop zone, base attacked, harvester attacked, enemy sensed
RadarEventDurations=400,400,400,400,600,600
```

An event is deleted once this timer has run out and its box has stopped turning. Until then it stays in the list the suppression test scans. For the three suppressible kinds the value therefore also sets how long the event goes on swallowing later events of its kind through [`RadarEventSuppressionDistances`](/keys/radareventsuppressiondistances/). It keeps swallowing them after [`RadarEventVisibilityDurations`](/keys/radareventvisibilitydurations/) has stopped it being drawn. An entry of zero or less is already expired when it is loaded, so that event is deleted on the frame it settles.

:::danger[The list must have an entry for all six kinds]
The lookup applies no bounds check, and any of the six kinds can index this list, so it needs six entries. Three kinds are raised by the engine and the other three only by the [Create Radar Event](/mapping/actions/taction-radar-event/) trigger action. An assignment with fewer gives the kinds past the end whatever integer happens to sit there, and a list left empty dereferences a null pointer. Either read happens the first time an event of an uncovered kind settles.
:::
