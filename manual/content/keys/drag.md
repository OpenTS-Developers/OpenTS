---
key: Drag
summary: The speed a levitating unit sheds each frame while it is coasting.
see_also: ["IntentionalDeacceleration", "Acceleration", "MaxVelocityWhenHappy", "Locomotor"]
when_omitted:
  kind: value
  value: "0.05"
---

A levitating unit travels in bursts: it thrusts for a fixed number of frames, coasts, brakes to a stop, and thrusts again. This figure is the loss applied on every frame of that cycle except two. The braking frame is governed by [`IntentionalDeacceleration`](/keys/intentionaldeacceleration/) instead, and the drifting frames apply no loss at all. It is subtracted from the unit's speed rather than scaling it, so the slowdown is a steady one and not a curve that tails off.

Once the loss is as large as the speed the unit is holding, its motion is zeroed on the spot rather than reversed. A figure at or above the speed a thrust reaches therefore leaves the unit stationary between thrusts.

Speeds and losses in this section are in leptons per frame (256 leptons to a cell, 15 frames to the second). A unit coasting at four leptons per frame comes to rest in eighty frames at a loss of `0.05` per frame, and in eight frames at `0.5`.

```ini title="rules.ini"
[LEVITATION]
Drag=0.1
MaxVelocityWhenHappy=5.0

[MYFLOATER] ; a UnitType registered in [VehicleTypes]
Locomotor={3DC0B295-6546-11D3-80B0-00902792494C} ; the levitation drive
```

The section governs only objects whose [`Locomotor`](/keys/locomotor/) is that identifier. The jumpjet drive's figures live in a separate `[JumpjetControls]` section and never reach a levitating unit; [what each locomotor drives its speed from](/systems/movement-and-terrain/#what-each-locomotor-drives-its-speed-from) gives the full mapping. Its values are held once for the whole game rather than per object, so a change to any of them moves every levitating object at once.

The whole section is read only from a file that also contains a `[General]` section. The read is made from inside the routine that handles `[General]`, and that routine returns without doing anything when the file has no such section. A map overriding any `[LEVITATION]` value must therefore contain a `[General]` section of its own, holding at least one assignment. A section with no entries in it is discarded as the file is read.
