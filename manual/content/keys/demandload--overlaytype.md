---
key: DemandLoad
scope: overlaytype
label: Overlay shape
see_also: ["Image", "NewTheater", "Theater"]
when_omitted:
  kind: value
  value: "no"
---

The flag detaches the archive shape already found under the overlay's [Image ID](/keys/image/), both after settings and save loading. The first draw loads a private copy.

An ordinary overlay loads its Image ID with a `.SHP` extension; [`Theater=yes`](/keys/theater/) uses the theater extension, while [`NewTheater=yes`](/keys/newtheater/) rewrites the ordinary name for the theater.

The copy is released with the type and, for theater-aware overlays, on theater changes.
