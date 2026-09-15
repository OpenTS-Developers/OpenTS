---
key: RadarEventVisibilityDurations
summary: The frames a settled radar event stays drawn, one entry per event kind.
see_also: ["system:map-visibility", RadarEventDurations]
when_omitted:
  kind: value
  value: ""
  note: The list stays empty and an event's own kind is used to index it anyway, dereferencing a null pointer.
---

Entries are positional: one per [radar event](/reference/enums/radar-event/) kind, in the order that page lists them, and an event indexes the list with its own kind. Entries are game frames, 900 to the game minute.

A radar event is drawn first as a box that shrinks toward the cell it flags, turning as it closes. This timer is loaded on the frame that box settles square, and it covers only the still marker that follows. The box closing in is drawn whatever this list says.

Expiry does not retire the event. It stops updating and stops being drawn, but it stays in play until [`RadarEventDurations`](/keys/radareventdurations/) runs out too. While it lingers, [`RadarEventSuppressionDistances`](/keys/radareventsuppressiondistances/) still measures a new event of its kind against it. When the duration is the shorter of the two, the event is deleted first and the rest of this timer never shows. The marker is drawn for whichever of the two is smaller.

:::danger[The list must have an entry for all six kinds]
Any of the six kinds can index this list, and the lookup applies no bounds check, so all six need an entry. Three kinds are raised by the engine and the other three only by the Radar Event trigger action. A kind that indexes past the end reads whatever integer sits there, and its marker either vanishes on the frame it settles or holds until its duration deletes it. A list left empty dereferences a null pointer.
:::
