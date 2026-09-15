---
key: VoiceMove
summary: The responses an object speaks when the player gives it an order other than an attack.
see_also: [VoiceAttack, VoiceSelect, VoiceDie, VoiceFeedback]
when_omitted:
  kind: value
  value: ""
---

```ini title="rules.ini"
[MYTANK] ; a UnitType registered in [VehicleTypes]
VoiceMove=MYTANK_Move1,MYTANK_Move2 ; sound IDs registered in SOUND.INI
```

This list covers every player-issued order that is not an attack: moving, guarding, harvesting, entering a transport, deploying. The acknowledgement is chosen on a single test for the attack mission, and everything else falls here. [`VoiceAttack`](/keys/voiceattack/) is the other side of that test.

One entry is picked at random and played at full volume, without distance fading. Ordering a group produces one response, not one per object: the first object to act speaks and the rest of that order is silenced. Names are matched and picked as [`VoiceSelect`](/keys/voiceselect/) describes.
