---
key: Crater
scope: smudgetype
label: Crater smudge
see_also: ["Burn", "Width", "Height", "Craters"]
when_omitted:
  kind: value
  value: "no"
---

The flag is what puts a smudge type into the pool a crater is chosen from. Nothing else does: [`Craters`](/keys/craters/) covers the list that looks as though it should and does not.

The pool is gathered afresh every time the ground is cratered. Every registered smudge type with the flag is offered the spot, those whose block will not fit there are dropped, and one of the survivors is picked at random. [`Height`](/keys/height/#scope-smudgetype) covers the fit test and the size grouping that narrows the survivors further.

Two things ask for a crater. An animation with [`Crater=yes`](/keys/crater/#scope-animtype) asks as it reaches its widest frame, after stripping up to six growth stages of [Tiberium](/systems/tiberium/) from the cell. It craters on every occurrence unless the animation also sets [`Scorch=yes`](/keys/scorch/), which turns each occurrence into a coin flip between the two pools.

A structure coming down asks in the two ways [`Burn`](/keys/burn/) describes, each of them a coin flip every time.

```ini title="rules.ini"
[MYCRATER]     ; example two-by-two crater
Crater=yes
Width=2
Height=2
```

The flag is independent of [`Burn`](/keys/burn/) on a smudge type. A type with both belongs to both pools, and a type with neither is registered, is given artwork, and is never picked for a scorch or a crater. A map file can still place such a type directly, through its `[Smudge]` list.
