---
key: VoiceDie
summary: The responses an object speaks as it is destroyed.
see_also: [VoiceFeedback, VoiceSelect, VoiceMove, VoiceAttack, MaxDebris]
when_omitted:
  kind: value
  value: ""
---

```ini title="rules.ini"
[MYTANK] ; a UnitType registered in [VehicleTypes]
VoiceDie=MYTANK_Die1,MYTANK_Die2 ; sound IDs registered in SOUND.INI
```

The response comes first on the destruction path, ahead of the object's wreckage, its explosion animation and any [`Explodes=yes`](/keys/explodes/#scope-aircrafttype) blast. Unlike the order acknowledgements, it is played at the object's own position and fades with distance from the visible screen. There is no test of whose house the object belongs to: an enemy object dying in view is heard exactly as a friendly one is.

One entry is picked at random. Names are matched and picked as [`VoiceSelect`](/keys/voiceselect/) describes.
