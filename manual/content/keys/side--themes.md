---
key: Side
scope: themes
label: Score side restriction
see_also: [Normal, Scenario]
when_omitted:
  kind: value
  value: <none>
  note: No restriction, which offers the score to every side.
---

The value is a side identifier. A score that names one is offered only while the local player's country belongs to that side; the check is against the country's [`Side=`](/keys/side/#scope-housetype), not against the country itself, so it covers every country grouped under the same side. Writing `<none>` leaves the score unrestricted, exactly as omitting the key does.

```ini title="theme.ini"
[DUSKHOUR]
Name=Dusk Hour
Length=4.11
Side=GDI
```

The restriction applies to the automatic playlist and to the sound options track list alike; a score can still be started outright by name whatever this is set to. Before a player's house has been settled the test is skipped, so an early score is not withheld for want of a side to compare against.

A value naming no declared side is logged and ignored, which leaves the score unrestricted.
