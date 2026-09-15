---
key: CrateRadius
summary: The distance from an opened crate within which its sweeping results reach other objects, in cells.
see_also: ["system:veterancy"]
when_omitted:
  kind: value
  value: "2.5"
  note: Fractions are accepted; the default reaches 640 leptons at 256 leptons to the cell.
---

Several crate results are not confined to the object that opened the crate. The cloaking, veterancy, armor, firepower, and speed results each sweep the ground layer. They apply themselves to objects whose center lies strictly nearer to the center of the crate's cell than this distance, the [veterancy result](/systems/veterancy/#promotion-without-kills) among them. The heal result also reaches past the collector, but it restores the collector's whole house and reads no distance at all.

The veterancy and cloaking results set no condition on the owner. Objects belonging to other houses, allied or hostile, are treated exactly like the collector's own, and buildings standing inside the circle are included alongside vehicles and infantry. The armor, firepower, and speed results skip any object whose matching multiplier an earlier crate has already changed. The speed result reaches only infantry and vehicles, never aircraft or buildings.
