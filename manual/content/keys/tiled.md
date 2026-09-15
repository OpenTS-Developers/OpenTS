---
key: Tiled
summary: Repeats the animation's current frame upward until the copies leave the top of the view.
see_also: ["Flat", "AltPalette", "Surface", "YDrawOffset"]
when_omitted:
  kind: value
  value: "no"
---

The frame the animation has reached is drawn again and again up the screen. Each copy sits one frame-height above the last, and the first sits half a frame-height above the animation's own position. The repetition stops once a copy has been placed above the top edge of the view. A column of smoke written this way reaches the top of the screen at any resolution rather than at the one it was drawn for. Every copy shows the same frame, so the column moves as one.

The step between copies is the height the shape file's header records for the first frame specifically, not the shape set's overall height. A shape whose frames vary in height tiles by whatever the first frame records.

This is a separate drawing path from the ordinary one, and three things do not survive the change:

- Every copy is drawn through the shared animation palette, so [`AltPalette=yes`](/keys/altpalette/), a structure's house colors and the tinted terrain palette of the cell below are all ignored.
- The copies are clipped against the whole tactical view rather than against the rectangle the animation was asked to draw into.
- [`Flat=yes`](/keys/flat/#scope-animtype) is never reached, because the repeated path is taken first.

The animation's translucency does survive: every copy is drawn at the level the animation had reached.

:::danger[A first frame that measures zero pixels tall hangs the game]
The copies step upward by the height the header records for the first frame, and the loop ends only once a copy has been placed above the top edge. A first frame recorded as zero pixels tall never moves them along, so the same copy is drawn in the same place without end and the game stops responding. A shape holding no frames at all records the same way, since the record of a frame the shape does not have comes back as nothing.
:::
