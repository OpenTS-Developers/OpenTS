---
key: StopSound
summary: Sound acknowledging the stop command.
see_also: [GuardSound, ScatterSound, DeploySound]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
StopSound=STOPCMD ; a sound ID registered in SOUND.INI
```

The [Stop Object](/commands/stopobject/) command plays this once after it has walked the selection, at full volume rather than faded from a place on the map. One sound covers the whole selection however many objects were in it.

It is the sound of the key being pressed rather than of an order being taken. The command only sends its stop event for an object the player may move or fire, but the sound is played whenever the selection holds anything at all. A selection of objects that all refused the order is heard exactly like one that took it. Only an empty selection is silent.
