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

With `DemandLoad=yes`, the overlay's shape is loaded when a scenario starts or a saved game is loaded, rather than with the rest of the rules, and stays loaded until the next scenario or saved game is loaded. The file can be loose or in any archive.

A demand-loaded shape is a separate copy for each overlay type. Two overlay types with the same [Image ID](/keys/image/) each load their own copy.

The file name is formed the same way as for any overlay. An ordinary overlay loads its Image ID with a `.SHP` extension. [`Theater=yes`](/keys/theater/) uses the theater's extension instead, and [`NewTheater=yes`](/keys/newtheater/) rewrites the `.SHP` name for the theater.
