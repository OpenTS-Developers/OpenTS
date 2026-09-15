---
key: IonStorms
summary: Parsed flag that gates nothing.
no_effect: true
see_also: ["system:ion-storms"]
when_omitted:
  kind: value
  value: "no"
  note: The special options are initialized with this built-in default when the game starts.
---

The flag is loaded with the rest of the scenario's special flags, but nothing in the game reads it. [Scripted storms](/systems/ion-storms/#starting-a-storm) run in a scenario whether it is set or cleared, and setting it does not make storms occur on their own.
