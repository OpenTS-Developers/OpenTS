---
key: DeploySound
scope: global-rules
label: Deploy order sound
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
DeploySound=27-I002 ; a sound ID registered in SOUND.INI
```

The [Deploy Object](/commands/deployobject/) command plays this once after it has issued the order. It sounds at full volume rather than faded from a place on the map. One sound covers the whole selection however many objects took the order, and a selection where none of them accepted it is silent.

This is the sound of the order rather than of the deployment. A structure's own [build-up sound](/keys/deploysound/#scope-buildingtype) is a separate setting on a separate slot.
