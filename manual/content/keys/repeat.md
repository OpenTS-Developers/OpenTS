---
key: Repeat
summary: Makes the score start again when it ends instead of handing over to the next one.
see_also: [Normal, Scenario]
when_omitted:
  kind: value
  value: "no"
---

```ini title="theme.ini"
[INTRO]
Name=Intro
Repeat=yes
```

Two places read it, and between them they close the loop. Starting the score queues the same score as the one to play next, and the routine that would otherwise advance the playlist hands back the score it was given rather than choosing another. The score therefore plays until something else replaces it, and neither the sequential order nor the shuffle ever moves off it.

The loop closes only around a score the game actually holds. A score whose audio file is missing from the mixfiles is advanced past like any other, rather than being handed back and asked for again.

The repeat button on the sound options screen does the same thing for every score at once. Either is enough on its own: a score marked this way repeats whether or not the button is on.
