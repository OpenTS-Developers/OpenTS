---
key: DemandLoad
scope: animtype
label: Animation demand loading
see_also: ["Image", "NewTheater", "Theater"]
when_omitted:
  kind: value
  value: "no"
---

The animation's shape file is loaded when a scenario starts or a saved game is loaded, rather than with the rest of the art, and stays loaded until the next scenario or saved game is loaded. The file can be loose or in any archive. Art that is not demand-loaded is read only from [cached archives](/formats/mix/#caching).

From the shape, the type takes its largest frame, on which a [`Crater`](/keys/crater/#scope-animtype), [`Scorch`](/keys/scorch/) or [`Flamer`](/keys/flamer/) animation acts. An [`End`](/keys/end/) that is omitted, `0` or `-1` plays the shape's frame count. Every player in a network game loads the shape at the same point, so all of them use the same values. When the file is missing, the largest frame is frame 0, such an `End` counts zero stages, and the animation draws nothing.

The file name is the [Image ID](/keys/image/#scope-animtype) with a `.SHP` extension, or the AnimType ID when no image is set. [`Theater=yes`](/keys/theater/#scope-animtype) uses the AnimType ID with the theater's extension instead, and [`NewTheater=yes`](/keys/newtheater/#scope-animtype) adjusts the ordinary name for the theater.
