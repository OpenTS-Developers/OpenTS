---
key: NewTheater
scope: animtype
label: Theater animation naming
see_also: ["Theater", "Image"]
when_omitted:
  kind: value
  value: "no"
---

The file keeps its `.SHP` extension and the second letter of its name is rewritten to the theater's [`ImageLetter`](/keys/imageletter/): `T` in temperate, `A` in snow. A name is rewritten only where its second letter is already the image letter of some declared theater, compared without regard to case. `GACNST` and `XTCNST` both follow the convention; `CITY01` does not and is left as written, so it loads the same file in every theater.

Unlike [`Theater=yes`](/keys/theater/#scope-animtype), this path does honor the animation's [`Image=`](/keys/image/#scope-animtype) and rewrites that name rather than the AnimType ID. The AnimType ID is used only when no Image ID is set.

:::caution[Artwork already in hand is not replaced]
The rename is reached only while the animation still has no shape. One that already found a file under its unrewritten name keeps it, so providing that name alongside the theater variants leaves the variants unused. The animation is loaded under its plain name as soon as its type is created, so a type whose unrewritten file exists never reaches the rename at all. One that did load a theater variant keeps that variant when the scenario theater later changes.
:::
