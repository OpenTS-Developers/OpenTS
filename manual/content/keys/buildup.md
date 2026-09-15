---
key: Buildup
summary: The construction animation file a structure plays while it is being built.
see_also: ["BuildupTime", "DemandLoadBuildup", "FreeBuildup", "GateStages", "Unsellable", "system:production"]
when_omitted:
  kind: value
  value: ""
  note: The type has no construction artwork.
---

The value is a filename without its extension. Only a non-empty value is taken, and only the first 15 characters of it are kept. The named `<value>.SHP` is loaded with the rules, after the name has been rewritten for the scenario theater on the structure-art convention that [`DoorAnim`](/keys/dooranim/) describes.

The file supplies both the frames of the construction animation and its length. Each step draws one frame of the file, starting at the first, and half the file's frames are the step count. A [`Gate=yes`](/keys/gate/) type instead takes the step count from [`GateStages`](/keys/gatestages/) plus one rather than from the file's size. [`BuildupTime`](/keys/builduptime/) is divided by that count to give the delay between one step and the next. [Buildup](/systems/production/#buildup) covers what happens around the animation while it runs.

```ini title="art.ini"
[MYWEAP] ; example war factory, drawn from its own Image ID
Buildup=GAWEAPMK ; loaded as GTWEAPMK.SHP in temperate
```

## A type with no construction artwork cannot be sold

Each structure asks its type for the construction artwork as it is created, and records the answer. A type that has none leaves every structure of that type with no answer to record, whether the value is missing or names a file that is not there. The sale is then refused however it is reached: the sell cursor, a trigger, and the computer's own emergency sell-off all pass through the same gate. [Capture](/systems/capture/) covers what a captured structure of such a type does instead. [`Unsellable=yes`](/keys/unsellable/) is the deliberate refusal, and it is the weaker one: the computer's sell-off and the trigger action both bypass that flag, and neither bypasses this gate.

The same answer decides one other thing: only a structure whose type has construction artwork turns a [`Nominal=yes`](/keys/nominal/) survivor into a technician as it is destroyed or sold.

:::caution[A theater-specific structure is timed differently]
A [`Theater=yes`](/keys/theater/) structure has its construction artwork fetched again as the theater is set up, this time under the theater's own extension rather than `.SHP`. That path counts every frame in the file as a step and fixes the delay at five seconds divided by that count, so `BuildupTime` does not reach it and neither does the halving above.
:::
