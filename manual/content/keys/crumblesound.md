---
key: CrumbleSound
summary: Sound a structure makes as its destruction sequence begins.
see_also: [BlowupSound, IsLimpetMine, "system:emp-pulse"]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
CrumbleSound=BLDGDIE1 ; a sound ID registered in SOUND.INI
```

The sound is played from the structure's own position, once, on the pass that tears the structure down. It comes after the cargo has been killed and the cloak generator or laser fence links have been shut off, and before the scorch marks, debris and survivors are placed. One sound covers every structure however large, and it is independent of the explosion animations the type names for itself.

Two paths reach that sequence: a structure damaged to destruction, and an [`IsLimpetMine=yes`](/keys/islimpetmine/) structure caught in an [EM pulse](/systems/emp-pulse/), which is destroyed outright rather than stunned. Selling a structure does not reach it. The build-down runs its own sequence and takes [`SellSound`](/keys/sellsound/) instead.
