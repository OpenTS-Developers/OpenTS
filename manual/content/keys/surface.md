---
key: Surface
summary: Places the animation in the ground layer, among the units and structures, rather than in the layer above them.
see_also: ["YSortAdjust", "Flat", "Tiled", "FlightLevel"]
when_omitted:
  kind: value
  value: "no"
---

The ground layer is the only display layer the engine keeps in order. An animation in it is sorted against the units, structures and terrain sharing it, and is drawn in its place among them. An animation left out of it joins the layer above. That layer is drawn after the whole ground layer and after the artwork that hangs off structures, so the animation is laid over that whole scene rather than sorted into it. Fire, smoke and the animations a structure runs want the ground layer; explosions and other effects that read as being in front of everything do not.

[`YSortAdjust`](/keys/ysortadjust/) has an effect only in the ground layer, since that is the only layer whose order is maintained.

An animation pinned to an object is in the ground layer whatever this says, so the flag matters only to an animation standing on its own.

The flag also picks a height for the animation as it is created: the rules' [`FlightLevel`](/keys/flightlevel/#scope-global-rules) when it is off, ground level when it is on. The coordinate the animation is placed at overwrites that choice a few lines later, so it never reaches the screen and no decision hangs on it. An animation sits at whatever height the code that created it asked for.
