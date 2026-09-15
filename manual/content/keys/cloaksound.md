---
key: CloakSound
summary: The sound played at an object's position as it starts to hide or to reappear.
see_also: ["system:cloaking"]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
CloakSound=Cloak1 ; a sound ID registered in SOUND.INI
```

The sound is played at the coordinate the object stands on, once at the start of each fade, for every object of every house. That includes each vehicle and infantryman a growing [cloaking field](/systems/cloaking/#cloaking-fields) passes over, and each structure that fades out under one. One start is silent: a critically damaged object (health at or below [`ConditionRed`](/keys/conditionred/)) still fading out abandons the cloak on a ten percent chance each frame, and the fade back into view begins without the sound.
