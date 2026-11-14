---
key: GateUpSound
summary: The sound a gate of this type plays as its door starts going up.
see_also: ["GateUp", "GateDownSound", "Gate", "system:walls-and-gates"]
when_omitted:
  kind: value
  value: none
  note: A gate with no sound of its own plays the [`GateUp`](/keys/gateup/) sound.
---

```ini title="rules.ini"
[MYGATE] ; BuildingType
Gate=yes
GateUpSound=GATEUP ; a sound ID registered in SOUND.INI
```

`GateUpSound=` names the sound a [`Gate=yes`](/keys/gate/) structure of this type plays as its door starts going up, in place of the default `[AudioVisual]` [`GateUp`](/keys/gateup/) sound. The paired key, [`GateDownSound`](/keys/gatedownsound/), sets the going down sound.
