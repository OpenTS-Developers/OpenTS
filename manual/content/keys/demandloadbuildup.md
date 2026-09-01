---
key: DemandLoadBuildup
summary: Defers loading the structure's construction artwork until a structure of the type is placed.
see_also: ["Buildup", "FreeBuildup", "DemandLoad"]
when_omitted:
  kind: value
  value: "no"
---

By default the file [`Buildup=`](/keys/buildup/) names is fetched from the archives as the rules are read, and again whenever a [`Theater=yes`](/keys/theater/) structure's theater is set up. With the flag set neither fetch happens. The file is instead read from disk the first time a structure of the type asks its type for the construction artwork, which is as that structure is created, and the block then belongs to the type and is released with it.

The deferred read builds the filename with a `.SHP` extension and the structure-art theater rewrite whatever the type's own theater settings say, so a `Theater=yes` structure that defers its construction artwork looks for the plain file rather than the theater-suffixed one. It gives the step count and the rate their ordinary treatment — half the frames in the file, divided into [`BuildupTime`](/keys/builduptime/) — where the theater fetch does not.

Only artwork read this way is released by [`FreeBuildup=yes`](/keys/freebuildup/). Without `DemandLoadBuildup=yes`, `FreeBuildup=yes` has no effect.
