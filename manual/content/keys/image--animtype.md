---
key: Image
scope: animtype
label: Animation Image ID
see_also: [End, Theater, NewTheater]
when_omitted:
  kind: computed
  note: Uses the AnimType ID as the animation Image ID.
---

An animation loads its artwork from `<AnimType ID>.SHP` as soon as its type is created. This setting takes effect only while the type holds no shape. Either the first load found no file under the AnimType ID, or the type sets [`DemandLoad=yes`](/keys/demandload/#scope-animtype) and loads the name given here on demand. In both cases the animation draws `<Image ID>.SHP`. An animation that never found a shape under either name draws nothing and is otherwise undisturbed.

The stage count comes from the first shape the type holds and is not re-measured when another shape loads. The [`End`](/keys/end/) key overrides the count. The two theater paths treat this setting differently in turn: [`Theater=yes`](/keys/theater/#scope-animtype) loads the artwork under a theater-suffixed name, while [`NewTheater=yes`](/keys/newtheater/#scope-animtype) keeps the name given here and rewrites its theater letter.
