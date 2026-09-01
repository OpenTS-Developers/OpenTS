---
key: DemandLoad
scope: buildingtype
label: Structure shape
see_also: ["DemandLoadBuildup", "Image", "Theater"]
when_omitted:
  kind: value
  value: "no"
---

A structure's shape is found in the archives under its [Image ID](/keys/image/) before this flag is read. The flag detaches that archive pointer, records the theater-resolved [main-shape basename](/keys/image/#scope-buildingtype), and leaves the type empty until its first draw loads a private copy.

The copy is released when the type's rules are reread, the type is destroyed, or theater setup revisits a `Theater=yes` or `NewTheater=yes` type. Unused types allocate nothing; a missing shape is not drawn.

The construction animation is a separate setting, [`DemandLoadBuildup`](/keys/demandloadbuildup/). The deploying, door, under-door, bib and Z-shape overlay artwork is fetched with the rules whatever this is set to.
