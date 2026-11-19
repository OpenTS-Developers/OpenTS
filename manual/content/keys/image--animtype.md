---
key: Image
scope: animtype
label: Animation Image ID
see_also: [End, Theater, NewTheater]
when_omitted:
  kind: computed
  note: Uses the AnimType ID as the animation Image ID.
---

The animation draws `<Image ID>.SHP` when that file exists, and otherwise `<AnimType ID>.SHP`. If neither file exists, it draws nothing. The same files are used in a new game and after a saved game is loaded. An animation with [`DemandLoad=yes`](/keys/demandload/#scope-animtype) loads `<Image ID>.SHP` when a scenario starts or a saved game is loaded, and has no fallback.

An [`End`](/keys/end/) that is omitted, `0` or `-1` plays the frame count of the shape the animation draws, so an `Image=` that selects a different shape changes it.

The theater settings treat this key differently:

- [`Theater=yes`](/keys/theater/#scope-animtype) names the artwork after the AnimType ID, so this key does not change which file is drawn.
- [`NewTheater=yes`](/keys/newtheater/#scope-animtype) rewrites the theater letter in the name given here.
