---
key: IonStormWarning
summary: Seconds of warning between a scripted ion storm being ordered and the storm breaking.
see_also: ["system:ion-storms"]
when_omitted:
  kind: value
  value: "31"
---

Both scripted ways of starting a storm pass this figure as the countdown, so it applies to every storm a scenario raises. The countdown runs down one frame at a time. Every fifteen seconds of it, the EVA approach line plays with a ten-second on-screen message, so a value of `31` produces two announcements, 30 and 15 seconds out.

```ini title="rules.ini"
[General]
IonStormWarning=31
```

At `0` the storm breaks on the frame its trigger action or team mission runs, with no approach warning. Only the break's own on-screen message shows. [The warning](/systems/ion-storms/#the-warning) covers what happens when a second storm is ordered while a countdown is already running.
