---
key: DemandLoad
scope: overlaytype
label: Overlay shape
see_also: ["Image", "NewTheater", "Theater"]
when_omitted:
  kind: value
  value: "no"
---

```ini title="art.ini"
[BTIB01]
DemandLoad=yes
```

The engine reads the flag from the overlay's `[<Image ID>]` section in `art.ini`, with `artfs.ini` read over it when Firestorm is enabled. The section is the one the overlay's [Image ID](/keys/image/) names.

With the flag set, the engine does not load the overlay's shape when it reads the settings. A saved game does not bring it back either. Shape data read from the archives does not survive a save, so when a saved game loads the engine fetches every overlay type's shape again, and a demand-loaded overlay is the one left out. The first request for the shape loads a private copy instead: the map load makes that request for every overlay the map places, and anything else waits for the first draw. The copy is held per overlay type, so two overlays pointed at one Image ID each load their own.

An ordinary overlay loads its Image ID with a `.SHP` extension. [`Theater=yes`](/keys/theater/) uses the theater extension instead, and [`NewTheater=yes`](/keys/newtheater/) rewrites the ordinary name for the theater.

The engine frees the copy when the type is discarded and, for overlays with [`Theater=yes`](/keys/theater/) or [`NewTheater=yes`](/keys/newtheater/), again on theater changes.
