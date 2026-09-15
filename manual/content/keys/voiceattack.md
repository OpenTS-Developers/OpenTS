---
key: VoiceAttack
summary: The responses an object speaks when the player orders it to attack.
see_also: [VoiceMove, VoiceSelect, VoiceDie, VoiceFeedback]
when_omitted:
  kind: value
  value: ""
---

```ini title="rules.ini"
[MYTANK] ; a UnitType registered in [VehicleTypes]
VoiceAttack=MYTANK_Atk1,MYTANK_Atk2 ; sound IDs registered in SOUND.INI
```

Only a player-issued attack order reaches this list. The acknowledgement is chosen before the order is carried out, on the mission alone, so it is spoken whether or not the object turns out to have a weapon that can reach the target. Every other order the player gives answers with [`VoiceMove`](/keys/voicemove/) instead. An attack an object starts on its own (retaliation, guarding, a computer house's orders) speaks nothing.

One entry is picked at random and played at full volume, without distance fading. Ordering a group produces one response, not one per object. Names are matched and picked as [`VoiceSelect`](/keys/voiceselect/) describes.
