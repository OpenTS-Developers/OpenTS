---
key: IsMeteor
scope: animtype
label: Animation meteor flight
see_also: ["Bouncer", "MinZVel", "MaxXYVel", "CraterLevel", "IsTiberium", "Spawns"]
when_omitted:
  kind: value
  value: "no"
---

The flag is one of the two that put an animation into the bounce physics at all; [`Bouncer`](/keys/bouncer/) is the other, and this one wins where both are set. An ordinary thrown animation is created where it was asked for and flung up from there. A meteor is created at the far end of a flight instead. A velocity is drawn, an approach of 51 to 70 frames is picked, and the animation is placed that many frames back along the velocity. It travels in and arrives at the coordinate it was created for.

The velocity itself is drawn differently in three ways.

- Its vertical component is [`MinZVel`](/keys/minzvel/#scope-animtype) exactly, with no random pick. The sign decides the approach: a negative figure brings the meteor down onto the target from above, a positive one brings it up from underneath.
- Its two horizontal components come from the [`MaxXYVel`](/keys/maxxyvel/#scope-animtype) range and are then mirrored together whenever `X` falls below `-Y`, which confines every meteor's approach to one half of the compass.
- Gravity is added back to the animation each frame after the physics have taken it away, so a meteor travels a straight line where other thrown animations arc.

Coming down on solid ground adds one effect an ordinary thrown animation never gets: the terrain is slumped around the impact cell according to [`CraterLevel`](/keys/craterlevel/). An end 416 leptons or more above the ground, the height of a bridge deck and about a cell and a half, is exempt from it, a bridge deck included. So is a landing in water, where the last animation of the rules' [`SplashList`](/keys/splashlist/) stands in for the wake and small splash other animations make there.

:::caution[The approach outlasts most animations]
The flight takes 51 to 70 frames and the animation keeps stepping through its own frames the whole way. One that reaches the end of them is removed in mid-air and never arrives, so a meteor type needs [`LoopCount=-1`](/keys/loopcount/) to loop until it lands.
:::
