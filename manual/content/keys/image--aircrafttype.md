---
key: Image
scope: aircrafttype
label: Image ID
see_also: [Voxel, Theater, NewTheater]
when_omitted:
  kind: computed
  note: Uses the ObjectType ID as the Image ID.
---

The value is both the basename of the artwork file and the name of the `art.ini` section the type's art settings are read from. Two types pointed at one Image ID therefore share the entry as well as the file. Whether that basename is taken as a `.SHP` or as a `.VXL` is decided by [`Voxel=`](/keys/voxel/) in that section. [`Theater=`](/keys/theater/#scope-aircrafttype) and [`NewTheater=`](/keys/newtheater/#scope-aircrafttype) are read from it as well. A shape that is not found leaves the type with no artwork rather than a substitute.

:::danger[A voxel animation whose `.VXL` is missing faults the first time one is drawn]
A VoxelAnimType with a section of its own loads `<Image ID>.VXL` as that section is read, unless it is set to [borrow another type's model](/keys/sharebodydata/). A file that is not there does not leave it empty-handed. The load hands back a model that is flagged as failed and holds no layers, and nothing in the animation's load or draw path reads that flag. The draw path tests only that a model is present. It then reads a layer record out of the table the failed load never allocated, so the first frame in which one of these animations is on screen faults. A VoxelAnimType that is only named by something else and never given a section of its own holds no model at all and is passed over. An aircraft in the same position is not faulted this way: its type's read throws the failed model away and leaves the aircraft with none.
:::
