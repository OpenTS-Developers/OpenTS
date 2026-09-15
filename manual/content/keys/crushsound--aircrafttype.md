---
key: CrushSound
scope: aircrafttype
label: Crushed sound
see_also: ["Crushable"]
when_omitted:
  kind: value
  value: none
---

The sound is played by the crusher, at the crusher's position, on the single frame the victim is destroyed. The value names a sound ID registered in [SOUND.INI](/formats/sound-ini/), read from the type being crushed rather than from the vehicle doing the crushing. It is the only sound either path plays: a crushed object and a flattened wall overlay both go quiet if their type names none.

Only a [`Crushable=yes`](/keys/crushable/#scope-aircrafttype) type ever reaches either path, so the setting is inert on anything a crusher cannot drive over.
