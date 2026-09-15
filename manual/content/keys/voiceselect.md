---
key: VoiceSelect
summary: The responses an object speaks when it is selected.
see_also: [VoiceMove, VoiceAttack, VoiceDie, VoiceFeedback, VoiceComment, Selectable]
when_omitted:
  kind: value
  value: ""
---

```ini title="rules.ini"
[MYTANK] ; a UnitType registered in [VehicleTypes]
VoiceSelect=MYTANK_Sel1,MYTANK_Sel2 ; sound IDs registered in SOUND.INI
```

One entry is picked from the list at random and played at full volume, not as a positional sound effect, so an object selected far off the visible screen is as loud as one under the cursor. The response is spoken only for a house the player controls.

Selecting several objects at once does not produce several responses. The first object that is actually selected speaks, and the rest of that selection is silenced, whether the objects were taken with a band box, a group key or a replayed recording.

Names are matched against the IDs registered in [SOUND.INI](/formats/sound-ini/); anything unmatched is dropped, so a list of nothing but misspellings leaves the type silent rather than falling back on anything. Writing the key with nothing after the `=` is a different thing: the read finds no value and keeps whatever an earlier rules file set. The other four voice lists are read the same way, and a space after a comma has the same effect here: the spaced name fails to match and is dropped.
