---
key: UndeploySound
summary: The sound a structure makes as its deconstruction starts.
see_also: [AuxSound2, DeploySound, IsJuggernaut, SellSound]
when_omitted:
  kind: value
  value: none
  note: The slot holds whatever [`AuxSound2`](/keys/auxsound2/) put there, and nothing at all when that key is unset too.
---

```ini title="rules.ini"
[MYUNDEPLOY] ; a BuildingType registered in [BuildingTypes]
UndeploySound=MYUP1 ; a sound ID registered in SOUND.INI
```

A structure plays this at its own position as a deconstruction begins (a sale or an undeploy), once its animations have stopped. For an [`IsJuggernaut=yes`](/keys/isjuggernaut/) structure it waits until the body and barrel have returned to [`StartFacing`](/keys/startfacing/) and [`StartPitch`](/keys/startpitch/). The reverse build-up follows it, so this is the opening sound of a sale rather than its closing one. The crew evacuation and the [`SellSound`](/keys/sellsound/) come after it too. A structure with an [`UndeploysInto`](/keys/undeploysinto/) vehicle puts out no crew. The game counts that structure as a vehicle, so it plays no `SellSound` either; a sold construction yard, which also has an `UndeploysInto`, still sounds it.

:::caution[It replaces `AuxSound2`]
The key stores into the same slot as [`AuxSound2`](/keys/auxsound2/) and is read after it, so a BuildingType that sets both keeps only this one. A name that matches no registered sound leaves the slot alone rather than clearing it.
:::
