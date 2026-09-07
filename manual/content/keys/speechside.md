---
key: SpeechSide
summary: The side whose voice set narrates a campaign mission.
see_also: [Player, RequiredAddOn]
when_omitted:
  kind: context-dependent
  note: The side of the country the scenario's Player entry names.
---

```ini title="map file"
[Basic]
Player=Nod
SpeechSide=GDI
```

The voice archive is chosen by the side's position in the rules `[Sides]` list plus one, so the first side draws `SPEECH01.MIX` and the second `SPEECH02.MIX`; any expansion voice packs registered for the same position are layered over it. The setting exists so that a mission fought as one side can be narrated by the other, which is how the stock Firestorm missions are voiced.

It is read only in a campaign mission, and only after the side has already been settled from [`Player`](/keys/player/#scope-scenarios-2), so it overrides that choice for speech alone. Art, interface and the buildable list continue to follow `Player`. Writing `<none>` is the same as leaving the key out.

The game ships voices for the first two sides only. A side with no voice archive of its own, the `Civilian` or `Mutant` side the stock rules also declare included, is narrated by the first side, and the load is abandoned only when the first side's archive cannot be found either. A name matching no side at all is logged and ignored, so the side settled from `Player` narrates.
