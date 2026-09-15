---
key: IonSensitive
summary: Stops the weapon from firing while an ion storm is running.
see_also: ["system:ion-storms"]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[MyRailgun] ; example WeaponType
IonSensitive=yes
```

The refusal is checked once the weapon itself has been resolved, so it applies to whichever object has the weapon and to both weapon slots. Nothing else about the weapon changes: reload, range, and ammunition are untouched, and the object may still fire its other weapon when that one is not marked.

Aircraft are barred from firing during [a storm](/systems/ion-storms/#aircraft) whatever their weapons set, so this key changes nothing for them.
