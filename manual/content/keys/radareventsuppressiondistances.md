---
key: RadarEventSuppressionDistances
summary: The cells within which a radar event swallows a later event of its kind, one entry per event kind.
see_also: ["system:map-visibility", RadarEventDurations]
when_omitted:
  kind: value
  value: ""
  note: The list stays empty and one of the three suppressible kinds is used to index it anyway, dereferencing a null pointer.
---

Entries are positional: one per [radar event](/reference/enums/radar-event/) kind, in the order that page lists them. Before an event is created the engine measures the straight-line cell distance from the proposed cell to every event of the same kind still in play. The proposal is dropped if any of them is strictly closer than that kind's own entry. Events stay in that scan after they have stopped being drawn, until [`RadarEventDurations`](/keys/radareventdurations/) removes them.

Only three kinds are tested this way: combat, harvester attacked and enemy sensed. The other three are always created, and the entries they would use are parsed and never read.

The distance also throttles three EVA lines across two events, because each is spoken only when its event is created rather than swallowed. The harvester-under-attack line follows a harvester-attacked event. That event is raised when a harvester-type unit of the local player's takes damage that registers a result and survives it, so a killing blow raises no event and speaks no line. The cloaked- and subterranean-detected lines follow an enemy-sensed event, raised as a sensed enemy that is cloaked or underground has its blip repositioned on the radar.

:::danger[The list must be six entries long]
The lookup applies no bounds check, and enemy sensed is the last of the six kinds. It is the kind that reads the last entry, so a list of five entries or fewer already misreads it. Entries past the end give whatever integer happens to sit there, and a list left empty dereferences a null pointer. Either read happens as soon as an event of a suppressible kind is proposed while one of that kind is still in play.
:::
