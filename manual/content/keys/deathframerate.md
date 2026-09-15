---
key: DeathFrameRate
summary: The number of game frames each frame of a vehicle's wreck animation is held for.
see_also: ["DeathFrames", "MaxDeathCounter", "StartDeathFrame"]
when_omitted:
  kind: value
  value: "1"
---
The frame of the run to draw is the wreck's death counter divided by this figure. The whole run therefore takes `DeathFrames × DeathFrameRate` game frames to play through (15 game frames to the second). Once it has played through, the last frame is held until the counter reaches [`MaxDeathCounter`](/keys/maxdeathcounter/) and the wreck explodes. A run that finishes early therefore leaves the wreck sitting on its final frame for the rest of its time. The count is kept in a single signed byte, so `256` stores as zero and any figure above `127` stores as a negative number.

The below-one check reads the stored byte rather than the figure on the line, and the store happens first: `DeathFrameRate=200` wraps to `-56` and is raised to `1` with the rest. Figures above `127` therefore mean a normal-speed wreck, not a very slow one. `DeathFrameRate=0` and negative figures hold each frame for one game frame too.
