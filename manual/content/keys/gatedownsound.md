---
key: GateDownSound
summary: The sound a gate of this type plays as its door starts going down.
see_also: ["GateDown", "GateUpSound", "Gate", "system:walls-and-gates"]
when_omitted:
  kind: value
  value: none
  note: A gate with no sound of its own plays the [`GateDown`](/keys/gatedown/) sound.
---

```ini title="rules.ini"
[MYGATE] ; BuildingType
Gate=yes
GateDownSound=GATEDOWN ; a sound ID registered in SOUND.INI
```

`GateDownSound=` names the sound a [`Gate=yes`](/keys/gate/) structure of this type plays as its door starts going down, in place of the default `[AudioVisual]` [`GateDown`](/keys/gatedown/) sound. The paired key, [`GateUpSound`](/keys/gateupsound/), sets the going up sound.
