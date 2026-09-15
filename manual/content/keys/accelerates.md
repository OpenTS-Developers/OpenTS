---
key: Accelerates
summary: Makes a driving vehicle ease up to speed and brake as it nears its destination.
see_also: ["AccelerationFactor", "DeaccelerationFactor", "SlowdownDistance"]
when_omitted:
  kind: value
  value: "yes"
---

Only the drive locomotor reads the flag. A type moved by any other [`Locomotor=`](/keys/locomotor/) travels at whatever speed its own locomotor asks for, whatever this says.

Cleared, the vehicle is set outright to the speed it has been told to hold, every step of the way: it reaches full speed in one frame and stops dead. Set, the speed is walked toward that target instead. [`AccelerationFactor`](/keys/accelerationfactor/) raises it while it is climbing and [`DeaccelerationFactor`](/keys/deaccelerationfactor/) lowers it while it is falling, with the braking run beginning within [`SlowdownDistance`](/keys/slowdowndistance/) of the destination.

The passive test sits inside this flag's own branch: a vehicle is passive when its type sets [`Passive=yes`](/keys/passive/). While the flag is set, nothing ramps a passive vehicle's speed, while `Accelerates=no` still sets its throttle outright every step. A passive vehicle that is following another is driven from the front instead: on every step, the front vehicle copies its own speed onto every vehicle following it. The clamp that holds a vehicle to a fifth of its speed while it is crushing something sits inside the ramping branch, so a vehicle without the flag crushes at full speed.
