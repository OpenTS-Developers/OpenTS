---
key: UseIonStorms
summary: Whether random map generation folds the ion storm file into the map it builds.
see_also: ["system:ion-storms"]
when_omitted:
  kind: value
  value: "no"
  note: The fallback of `no` leaves `ION.INI` unloaded, so generation registers no ion storm triggers.
---

```ini title="map seed file"
[RandomMap]
UseIonStorms=yes
```

With the option set, generation loads the hard-coded file `ION.INI` and takes three things from it. Its `[General]` section is applied over the loaded rules. The six ion values in its `[Lighting]` section replace the ones the generated map would otherwise have. Its trigger types and tag types are registered with the map. Those triggers are what actually [raises the storms](/systems/ion-storms/#random-maps); the option by itself schedules nothing.

The generator asks no questions about the Firestorm addon here, so a seed file may set the flag whether or not that addon is present. The dialog's randomize button sets the option on about half the seeds it produces, and only while the Firestorm addon is enabled. The map generator dialog exposes it as a checkbox and stores the answer here. `ION.INI` is read through the ordinary file layer, so a loose file with that name in the game directory overrides whatever an archive ships; [MIX archives](/formats/mix/) covers that lookup.
