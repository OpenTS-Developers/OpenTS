---
key: FiringSyncFrame1
summary: The countdown value at which the first round of a burst leaves a shape-drawn vehicle.
see_also: ["FiringSyncFrame2", "FiringFrames", "StartFiringFrame", "Burst"]
when_omitted:
  kind: value
  value: "-1"
---

Firing a vehicle's primary weapon starts a countdown at `2 × FiringFrames − 1` that falls by one each game frame, and the frame on screen is that countdown halved. This setting holds the countdown value at which the round is actually released, so it places the shot inside the animation rather than in front of it. `FiringSyncFrame1=8` releases while the run's frame 4 is drawn; because each frame is held for two game frames, `9` shows that same frame and releases one game frame earlier.

While the countdown is running and has not reached the value, the weapon reports itself as still rearming and the shot is held. On the frame the countdown matches, the round leaves without the rate-of-fire timer being read at all: the animation, not the timer, paces the shot.

```ini title="art.ini"
[DEFENDER] ; the Image ID of the stock Core Defender
FiringFrames=12   ; the countdown runs 23 down to 0
FiringSyncFrame1=8 ; first round at frame 4 of the run
FiringSyncFrame2=3 ; second round at frame 1
```

At the default `-1` the round is not tied to the animation at all: it leaves first and the countdown starts behind it. The setting reaches only a UnitType's primary weapon and only the first two rounds of a [`Burst`](/keys/burst/); a secondary weapon and every later round of a burst fire untied whatever is set here.

:::caution[A value the countdown never reaches stops the weapon]
The countdown only ever holds the values from `2 × FiringFrames − 1` down to `0`. A larger figure, or a negative one other than `-1`, is never matched, so the round is never released and the vehicle's primary weapon never fires at all. A vehicle with no [`FiringFrames`](/keys/firingframes/) has no countdown to gate against and fires normally whatever is set here.
:::
