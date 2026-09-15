---
key: IdleActionFrequency
summary: Base interval, in minutes, between one infantryman's idle actions.
see_also: [VoiceComment, Sequence, Fraidycat]
when_omitted:
  kind: value
  value: ".083"
---

```ini title="rules.ini"
[AudioVisual]
IdleActionFrequency=.083
```

Every time an infantryman performs an idle action it sets its own next one, drawing a random interval between half the value and twice it. The figure is therefore the middle of a spread rather than a fixed period. The value is in minutes at fifteen frames a second, which makes the default a wait of between two and a half and ten seconds. One figure covers the whole game, so lowering it makes every infantryman on the map fidget more often rather than picking out a type.

The wait is only checked while the infantryman is standing still with nothing worth shooting at, under a guard, hunt or guard-area mission. The interval is therefore a floor rather than a schedule: one under fire or on the move never reaches the check at all.

When it does fire, one of eleven equally likely outcomes is drawn. Three play the plain idle animation, three play an idle animation with a turn and an occasional muttered [`VoiceComment`](/keys/voicecomment/), four turn to a random facing, and one does nothing. A frightened [`Fraidycat=yes`](/keys/fraidycat/) infantryman under a computer house scatters instead of drawing at all.

Only infantry idle. Vehicles, aircraft and structures have no idle action and never read the value.
