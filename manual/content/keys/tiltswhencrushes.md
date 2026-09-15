---
key: TiltsWhenCrushes
summary: Makes a vehicle lurch forward as it crushes a sandbag wall.
see_also: ["Crusher"]
when_omitted:
  kind: value
  value: "yes"
---

The drive and mech locomotors both check the flag where they mark a vehicle as crushing. The mech checks the cell the vehicle is about to enter. The drive checks the cell it has just entered, and only while the move it is making counts as a crush. The locomotor flags the hull as rocking whenever it drives straight through a cell holding a crushable overlay instead of steering around it, so a tank rolling straight into the wall qualifies. In each case the cell holds sandbag wall and the vehicle is a [`Crusher=yes`](/keys/crusher/) or has earned the crusher ability. With the flag set, the hull takes a forward tilt at that moment, heavier under the drive locomotor than under the mech. Cleared, the wall still goes and the vehicle is still marked as crushing. That marking holds an [`Accelerates=yes`](/keys/accelerates/) vehicle to a fifth of its speed while it goes through, but the hull stays level.

Only sandbag wall triggers it. Every other crushable overlay rocks the vehicle through the ordinary crush path, which does not read this flag. The stock rules clear it on one type, the Mammoth Mk. II.
