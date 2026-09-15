---
key: Medians
summary: Fourteen-tile set of the divider strips laid down the middle lane of a paved road.
see_also: [ClearToPaveLat]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

The random map generator uses four of the fourteen, and only on the north-south road band; the east-west band lays no medians at all. Offset 5 is the head of a strip and offset 4 repeats down its body, both dropped in the center column as each straight road row goes down. Offset 3 caps the tail, placed either where a junction interrupts the run or where the band ends. Offset 12 is the stub laid on each crossing arm of a four-way junction, one cell to either side.

A strip is only started when more than five cells of road remain. One that would otherwise dangle past a plain end cap is walked back and overwritten with road again. The terrain test that walk follows is not gated on the role resolving: left unresolved, it accepts everything up to index 12, so the walk runs over the theater's first thirteen tiles and rewrites them as road.

Medians count as pavement when [`ClearToPaveLat`](/keys/cleartopavelat/) examines a neighbor, so a strip running through paved ground does not make that ground blend against it.
