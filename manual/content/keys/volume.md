---
key: Volume
summary: The share of full loudness the sound effect is played at.
see_also: [Priority, MinVolume, VShift, SoundVolume, VoiceVolume]
when_omitted:
  kind: value
  value: "1.0"
---

The value is a fraction of the sample's own loudness, and a value above 1 is read as a percentage, so `0.5` and `50` mean the same thing and `1` and `100` are both full loudness. The result is clamped between silence and full loudness; a sound cannot be made louder than its sample. The loudness the game asks for when it plays the sound, the distance fade of a placed sound, the random [`VShift=`](/keys/vshift/) and the player's [`SoundVolume`](/keys/soundvolume/) setting all scale it further, and the sound effect path falls silent altogether when that setting is at zero. The voice path leaves the setting out.

```ini title="sound01.ini"
[SCOLD8]
Priority=75
Volume=0.5
```

`[Defaults]` may carry the key as the value every sound section omits.

:::caution[A value above 1 no longer raises the loudness]
Earlier releases multiplied by the value as written and clamped only the product, so `Volume=2` played a quiet sample at up to twice its loudness. The same line now means two percent. Remove such values or write `Volume=100`.
:::
