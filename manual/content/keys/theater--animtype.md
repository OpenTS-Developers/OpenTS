---
key: Theater
scope: animtype
label: Theater-specific animation artwork
see_also: ["NewTheater", "Image"]
when_omitted:
  kind: value
  value: "no"
---

The animation's shape file is named after its AnimType ID, with the extension set by the scenario theater's [`Suffix`](/keys/suffix/#scope-theater) in place of `.SHP`. By default that is `.TEM` in temperate and `.SNO` in snow.

```ini title="art.ini"
[MYBLAST] ; an AnimType ID
Theater=yes ; draws MYBLAST.TEM or MYBLAST.SNO
```

When the theater's file is missing, the animation draws `<AnimType ID>.SHP` instead, and nothing when that file is missing too. A [`DemandLoad=yes`](/keys/demandload/#scope-animtype) animation has no fallback and draws nothing.

The file is looked up again for every scenario and every loaded saved game, so its extension always matches the current theater.

An animation that also sets [`NewTheater=yes`](/keys/newtheater/#scope-animtype) uses this flag and ignores that one.

Put `Theater=yes` in the art section of the Image ID, which is the AnimType's own section when [`Image=`](/keys/image/#scope-animtype) is not set. When the Image ID refers to another section, the AnimType's own section is read for this key only if neither `<AnimType ID>.SHP` nor `<Image ID>.SHP` exists.

The file names do not use `Image=`: a `Theater=yes` animation whose Image ID differs from its AnimType ID still draws the files named after the AnimType ID.
