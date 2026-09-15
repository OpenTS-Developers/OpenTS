---
key: GenericBeep
summary: Sound acknowledging a volume slider move.
see_also: [GenericClick, SpeakDelay]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
GenericBeep=BEEP1 ; a sound ID registered in SOUND.INI
```

Three volume sliders play it as feedback while they are dragged. The music volume plays it only while no music is playing; a slider moved while a music track runs is silent. The sound effects volume plays it on every move. The speech volume plays it only outside a running game; inside one, moving it speaks a random GDI or Nod taunt instead, and only when nothing else is already speaking.
