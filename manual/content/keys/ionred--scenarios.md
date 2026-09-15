---
key: IonRed
scope: scenarios
label: Scenario lighting
see_also: [Red, IonGreen, IonBlue, "system:ion-storms"]
when_omitted:
  kind: context-dependent
  note: The value this scenario's Red key sets, read earlier in the same section. With neither key present that is 1, leaving the red channel unchanged.
---

The three ion tints are applied to every tinted terrain palette the map has built, and to every color scheme with more than one intensity level, on the frame the storm breaks. They are reversed on the frame it ends. Neither change is faded, so the whole map switches color in one step while its [ambient level](/keys/ionambient/) is still traveling. The values have the same meaning as the ordinary tint keys. Each multiplies its channel of every palette color: 1 leaves colors unchanged, 0 removes the channel, and values clamp to 0 through 2. During the storm the ion values replace the ordinary tints rather than adding to them: Red=.7 with IonRed=1.2 draws a 1.2 red channel, not 1.9.

Any tinted palette built while a storm is running also takes these values. An object or overlay that first appears mid-storm therefore matches the ones already on screen.
