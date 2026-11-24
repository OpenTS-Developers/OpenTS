---
key: NewTheater
scope: animtype
label: Theater animation naming
see_also: ["Theater", "Image"]
when_omitted:
  kind: value
  value: "no"
---

`NewTheater=yes` makes the animation draw a theater-specific shape when one exists. The file keeps its `.SHP` extension, and the second letter of its name is replaced by the scenario theater's [`ImageLetter`](/keys/imageletter/): `T` in temperate and `A` in snow. A name is rewritten only when its second letter is already the image letter of some declared theater, ignoring case. `GACNST` and `XTCNST` follow this convention. With the stock theaters `CITY01` does not, so it loads the same file in every theater.

The name rewritten is the one [`Image=`](/keys/image/#scope-animtype) gives, or the AnimType ID when no Image ID is set. [`Theater=yes`](/keys/theater/#scope-animtype) uses the AnimType ID instead.

The renamed file is used when it exists. Otherwise the animation draws an unrenamed file: `<AnimType ID>.SHP` when that exists, and `<Image ID>.SHP` after that.

```ini title="art.ini"
[GAFLAG] ; example AnimType ID, with no Image= set
NewTheater=yes ; draws GTFLAG.SHP in temperate, or GAFLAG.SHP when that file is missing
```

Put `NewTheater=yes` in the art section of the Image ID, which is the AnimType's own section when `Image=` is not set. When the Image ID names another section, the AnimType's own section is read for this key only if neither `<AnimType ID>.SHP` nor `<Image ID>.SHP` exists.

:::caution[A demand-loaded animation gets no fallback]
A [`DemandLoad=yes`](/keys/demandload/#scope-animtype) animation draws nothing when the renamed file is missing, because the unrenamed files are never tried.
:::
