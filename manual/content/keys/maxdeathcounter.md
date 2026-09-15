---
key: MaxDeathCounter
summary: The number of game frames a destroyed shape-drawn vehicle lingers as a wreck before it explodes.
see_also: ["DeathFrames", "DeathFrameRate", "StartDeathFrame"]
when_omitted:
  kind: computed
  note: The derived StartDeathFrame plus DeathFrames, or -1 for a vehicle with no death frames.
---

A wreck's death counter starts at zero on the update that destroys the vehicle and steps up by one each game frame. Once it reaches this figure the wreck explodes and is deleted, so the figure is a duration in game frames (15 to the second) rather than a frame number.

The figure is independent of how long the animation itself runs, which is `DeathFrames × DeathFrameRate` game frames. The derived default is a frame number carried over rather than a duration chosen for the artwork: it is the frame just past the end of the death run. At the default [`DeathFrameRate`](/keys/deathframerate/) the run therefore always finishes first, and the wreck sits on its last frame for the remainder.

Only a vehicle with [`DeathFrames`](/keys/deathframes/) above `0` ever starts a death counter, so this setting is inert on any other vehicle.
