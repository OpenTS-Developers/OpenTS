---
key: CurleyShuffle
summary: Whether an attacking aircraft picks a fresh firing position between shots.
when_omitted:
  kind: value
  value: "no"
---

With `CurleyShuffle=no`, an aircraft that has reached a firing position with its target in range stays there and keeps firing while it holds both ammunition and a target. With `CurleyShuffle=yes` it fires twice from each position, then chooses a new one, flies there, and starts again.

The flag is read only inside the attack mission, and only on the branches where the aircraft is already in range. An aircraft that is out of range chooses a fresh position either way. A strafing run never reads the flag: its five shots are spaced by the weapon's rate of fire.
