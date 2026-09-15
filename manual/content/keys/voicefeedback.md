---
key: VoiceFeedback
summary: The responses an object speaks on the hit that takes it below half strength.
see_also: [VoiceDie, VoiceSelect, VoiceMove, VoiceAttack, DamageParticleSystems]
when_omitted:
  kind: value
  value: ""
---

```ini title="rules.ini"
[MYTANK] ; a UnitType registered in [VehicleTypes]
VoiceFeedback=MYTANK_Hurt1 ; a sound ID registered in SOUND.INI
```

Only one kind of hit reaches this list: the single blow that takes the object from at or above half its maximum strength to below it. The list stays silent when that blow also takes the object into condition red: that crossing is classified as its own result instead of the half-strength one, and it has no voice, so the biggest hits are the quietest. That hit then speaks on a 30% roll, so most objects cross the threshold silently, and further damage below the threshold says nothing at all. The transition is measured per hit, so repairing back above half strength arms the response again.

One entry is picked at random and played at the object's center, fading with distance from the visible screen; there is no test of whose house the object belongs to. Names are matched and picked as [`VoiceSelect`](/keys/voiceselect/) describes.
