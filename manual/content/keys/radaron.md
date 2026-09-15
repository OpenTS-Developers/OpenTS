---
key: RadarOn
summary: Sound played as the radar pane begins opening.
see_also: ["system:map-visibility", RadarOff]
when_omitted:
  kind: value
  value: none
---

The sound plays without a position, at the moment the pane is put into its opening state. The branch that plays it is skipped when the pane is already open or already opening. It does not repeat over the forty frames the opening animation runs for. [Power output and drain](/systems/power/#radar) covers what raises the pane in the first place.
