---
key: IsDropship
summary: Flies an aircraft as a heavy transport that noses over on approach instead of banking.
see_also: ["SlowdownDistance", "PitchAngle", "FlightLevel", "Dock"]
when_omitted:
  kind: value
  value: "no"
---

The fly locomotor treats such an aircraft differently in a handful of places, nearly all of them about how it comes down.

Its attitude is worked out even while it is sitting on the ground; an ordinary aircraft's attitude code runs only in the air, so on the ground it is drawn level. It never banks into a turn and never holds the nose-down attitude other aircraft take at speed. It tips over as it comes in instead, reaching its full [`PitchAngle`](/keys/pitchangle/) four tenths of the way through its [`SlowdownDistance`](/keys/slowdowndistance/), and levels out again at 0.02 radians per frame once it is down. Its cruising height drops to a third of its [`FlightLevel`](/keys/flightlevel/#scope-aircrafttype) as it crosses that same distance.

It does not bob gently in the air the way other aircraft do, and its shadow is laid flat rather than over the slope of the ground beneath it. Coming down it plays the `DROPLAND` animation below 300 leptons where a carryall plays `CARYLAND`.

Left without orders while airborne it goes looking for somewhere to put down: the first building from its [`Dock`](/keys/dock/) list that has a free bay, or a clear landing zone if none has. Before it lands on a spot, whatever is standing there that is not a structure is made to scatter.
