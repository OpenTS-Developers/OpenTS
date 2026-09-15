---
key: BombParachute
summary: Parsed canopy animation that nothing ever falls under.
no_effect: true
see_also: [Parachute, ChuteSound]
when_omitted:
  kind: value
  value: none
---

The paradrop routine chooses between two canopies by what is falling: a projectile takes this one and everything else takes [`Parachute`](/keys/parachute/).

Only one path reaches that routine. An aircraft carrying passengers drops them instead of firing its weapon, and what it hands over is a passenger detached from its cargo hold: always a vehicle, an infantryman or another aircraft, never a projectile. Nothing else in the game paradrops anything, so the branch this animation sits behind is never taken and the value is inert however it is set.
