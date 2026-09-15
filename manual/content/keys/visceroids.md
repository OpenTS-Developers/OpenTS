---
key: Visceroids
summary: Parsed flag that spawns nothing.
no_effect: true
see_also: [TiberiumDeathToVisceroid, SmallVisceroid]
when_omitted:
  kind: context-dependent
  note: Off for the session's first scenario. From the second scenario on, the read falls back to whatever an earlier scenario's SpecialFlags section wrote, since the flag is cleared only once when the session's special options are created.
---

The name promises the small visceroids that rise from infantry killed in Tiberium. The flag is parsed from the scenario's `[SpecialFlags]` section, and no gameplay path reads it. The switch that actually decides whether those deaths leave a visceroid behind is [`TiberiumDeathToVisceroid`](/keys/tiberiumdeathtovisceroid/) in the map's `[Basic]` section, which defaults to on when a map omits it.
