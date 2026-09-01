---
key: DemandLoad
scope: animtype
label: Animation shape
see_also: ["FreeAfterPlaying", "Image", "NewTheater", "Theater"]
when_omitted:
  kind: value
  value: "no"
---

The flag detaches the archive shape already found under the animation's [Image ID](/keys/image/#scope-animtype), both after settings and save loading. The first draw loads a private copy and fills any unset frame count or loop end.

That read uses the Image ID, or the AnimType ID when none is set, with a `.SHP` extension. [`Theater=yes`](/keys/theater/#scope-animtype) instead uses the AnimType ID and theater extension; [`NewTheater=yes`](/keys/newtheater/#scope-animtype) rewrites the ordinary name for the theater.

The private copy is released with the type, on theater changes for theater-aware animations, and after playback with [`FreeAfterPlaying=yes`](/keys/freeafterplaying/) — the only in-match release.
