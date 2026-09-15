---
key: PropulsionSoundEffect
summary: The sounds a levitating unit's thrust may play.
see_also: ["AccelerationProbability", "AccelerationDuration"]
when_omitted:
  kind: value
  value: ""
---

One entry is picked at random from the list and played at full volume, with no attenuation for how far the unit stands from the view. It plays on every fourth thrust rather than every one. The count that decides which thrust is the fourth is shared by every levitating unit in the match, so a map crowded with them plays the sound as often overall but rarely for any one unit.

```ini title="rules.ini"
[LEVITATION]
PropulsionSoundEffect=MYFLOAT1,MYFLOAT2 ; sound IDs registered in SOUND.INI
```

An entry naming a sound the game does not know is dropped as the list is read rather than kept as a silent slot. An empty list is checked for, so leaving the key unwritten costs the thrust its sound and nothing else.

[`Drag`](/keys/drag/) covers which objects read this section and the `[General]` section a file must contain for any of it to be read at all.
