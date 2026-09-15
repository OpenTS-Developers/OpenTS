---
key: ROT
scope: aircrafttype
label: Object rate of turn
when_omitted:
  kind: value
  value: "0"
---

The figure is a count of 256ths of a full circle stepped through per game frame, clamped to 127. A turn is timed by dividing its arc by that step, and the widest arc anything turns through is a half circle. A full about-face therefore takes 128 divided by the figure in frames: the stock `ROT=5` of a dropship comes to 25 frames, a little under two seconds. Both the body facing and the turret facing are set from the same figure.

```ini title="rules.ini"
[MYDROP] ; an AircraftType registered in [AircraftTypes]
ROT=5
```

A figure of zero (and any negative figure, which the clamp leaves alone) means the facing never turns at all. It snaps to whatever direction it is given the moment it is given, and the object always reads as facing where it wants to face.

Infantry are the exception: an infantry instance has its rate fixed at the maximum of 127 as it is created, whatever its type sets here. Healing damage and a repair over the radio both replace that with the type's own figure. An infantry type that leaves the key out therefore stops turning smoothly once it has been healed.

:::caution[A tunnelling type at zero burrows instantly]
The tunnel locomotor times both its dig-in and its surfacing by dividing 64 by this figure, and the dig-in figure is divided by [`TunnelSpeed`](/keys/tunnelspeed/) as well. A type with [`Locomotor={4A582743-9839-11D1-B709-00A024DDAFD1}`](/keys/locomotor/) therefore divides by zero when it burrows. The division is a floating-point one, so nothing breaks: both rotations report themselves complete the instant they begin. Both stock burrowers set a figure of their own.
:::

When an object fires a warhead with a limpet factor, the vehicle it hits loses top speed in proportion to the factor, and the firer removes itself from the game in the same step. The code applies its turn-rate reduction to that firer rather than to the target. The reduction therefore lands on an object that no longer exists: no object's rate of turn is ever changed, and the target keeps the turn rate its own type gives it and loses only speed.
