---
key: ZoomInFactor
summary: How far the tactical view magnifies when a trigger zooms it in.
when_omitted:
  kind: value
  value: "2"
---

The [Zoom in](/mapping/actions/taction-zoom-in/) trigger action stores this figure as the view's magnification, and [Zoom out](/mapping/actions/taction-zoom-out/) puts it back to `1`. While the magnification is anything other than `1`, each finished frame is presented from a centered rectangle of the rendered surface whose width and height are the full size divided by the figure, stretched back across the window. At `2`, the view shows a quarter of the area at twice the size.

Writing `1` leaves the zoom action with nothing to change on screen. It still locks input out, hides the cursor and stalls for a second, and control comes back when the matching zoom-out action runs, or when anything else hands input back to the player. The figure is used as a divisor with no zero guard, so `ZoomInFactor=0` leaves the source rectangle with no defined size.

The [Change Zoom Level...](/mapping/actions/taction-change-zoom/) trigger action does not read this figure; it changes nothing.
