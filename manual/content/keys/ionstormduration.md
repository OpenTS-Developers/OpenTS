---
key: IonStormDuration
summary: Parsed storm length that the engine never uses.
no_effect: true
see_also: [IonStormWarning, "system:ion-storms"]
when_omitted:
  kind: value
  value: "20"
---

No code reads the stored value, and nothing falls back to it. A storm's length comes only from the number named by the [Ion Storm start...](/mapping/actions/taction-ion-storm-start/) trigger action or the [Ion storm start in...](/mapping/missions/tmission-ion-storm-start/) team mission.

[`IonStormWarning`](/keys/ionstormwarning/) in the same section is the ion storm timing value the engine does read.
