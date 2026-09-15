---
key: VoiceComment
summary: The idle remark and the delivered remark a soldier speaks, in that order.
see_also: [Passengers]
when_omitted:
  kind: value
  value: ""
---

Only the first two entries are ever played, and each has its own trigger.

| Entry | Played when |
| --- | --- |
| First | The soldier starts its second idle animation; if it is unselected and owned by the local player, a one-in-three roll decides whether the remark is spoken |
| Second | The soldier has just been delivered and finishes its first step into a cell; the arrival cuts its tether to whatever built it or carried it |

```ini title="rules.ini"
[MYCIVILIAN] ; example InfantryType
VoiceComment=21-I000,21-I002 ; entries registered in the sound list
```

A soldier is delivered when it walks out of the building that built it or is set down by a transport aircraft, and either way the tether to that carrier is still in place. The first step the soldier completes cuts that tether, and the second entry plays at the same moment, once per delivery; a soldier tethered again on a later delivery speaks again. A ground transport puts infantry out with no tether, so passengers it drops say nothing.

The idle timer draws one of eleven outcomes each time it expires. Three of those outcomes run the second animation, and each of those three then rolls one-in-three for the voice, so the first entry is heard on roughly one idle in eleven. The voice does not depend on the animation existing: a type whose art has no second idle animation still speaks on the roll, with no animation to show for it. Both sounds play at the soldier's own position rather than through the speech channel. An entry naming a sound [SOUND.INI](/formats/sound-ini/) does not register is dropped as the list is parsed, which shifts every later entry down one place. Entries past the second are stored but never played.
