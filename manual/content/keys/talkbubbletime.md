---
key: TalkBubbleTime
summary: How long a scripted talk bubble stays above the unit that is speaking.
when_omitted:
  kind: value
  value: "5"
---

The figure is written in seconds, with fractions accepted, and multiplied by 60 as it is stored. The countdown it fills runs on the system clock rather than on game frames, and that clock advances 62.5 times a second. A bubble is therefore shown for slightly less time than the number written: `5` becomes 300 ticks, which run out after 4.8 seconds.

```ini title="rules.ini"
[General]
TalkBubbleTime=8  ; a bubble stays up for about 7.7 seconds
```

A bubble is placed by the [Talk Bubble](/mapping/missions/tmission-talk-bubble/) team mission. Placing one also reveals two cells of ground around the speaker, so every human player can see who is talking. Only one bubble exists at a time, so placing a second takes the first away. The [Talk Bubble...](/mapping/actions/taction-talk-bubble/) trigger action takes a bubble down rather than putting one up. Nothing but the drawing of the bubble reads the countdown, so a bubble that runs out stops being drawn.
