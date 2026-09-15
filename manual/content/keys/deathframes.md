---
key: DeathFrames
summary: The number of frames in the wreck animation a destroyed shape-drawn vehicle plays before it explodes.
see_also: ["StartDeathFrame", "DeathFrameRate", "MaxDeathCounter", "CrewEscape"]
when_omitted:
  kind: value
  value: "0"
---
Above `0`, the hit that destroys the vehicle no longer finishes it. The vehicle drops to one point of strength, stays on the map as a wreck that cannot move, and is exploded and removed once its death counter reaches [`MaxDeathCounter`](/keys/maxdeathcounter/). At `0` the destroying hit finishes the vehicle outright. The count is kept in a single signed byte, so `256` stores as that same zero and quietly turns the wreck off, while any figure above `127` stores as a negative number.

Nothing the ordinary destruction does happens on the wreck path: no death announcement, no passengers thrown clear, no wooden crate from a crate-carrying truck, and no crew survivor. [`CrewEscape`](/keys/crewescape/) covers the survivor roll that is passed over.

The count is a total rather than a per-facing figure. The frame drawn is [`StartDeathFrame`](/keys/startdeathframe/) plus the death counter divided by [`DeathFrameRate`](/keys/deathframerate/), held at the last frame of the run once that division reaches `DeathFrames − 1`. No facing term enters it, so the wreck shows the same run whichever way the vehicle was pointing.

```ini title="art.ini"
[MYWALKER] ; the Image ID of a shape-drawn UnitType
Facings=8
WalkFrames=12
DeathFrames=13     ; one run of 13, from frame 8 × (0 + 12 + 1) = 104
DeathFrameRate=3   ; 39 game frames of animation, about two and a half seconds
MaxDeathCounter=64 ; the wreck is removed after 64 game frames
```
