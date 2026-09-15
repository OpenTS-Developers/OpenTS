---
key: IsVeinhole
summary: Makes a veinhole monster built from the TerrainType selectable and targetable.
see_also: ["system:veins", "VeinholeTypeClass"]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[VEINTREE] ; the TerrainType named by [General] VeinholeTypeClass
Name=Veinhole Tree
Strength=1000
IsVeinhole=true
```

A [veinhole monster](/systems/veins/#veinhole-monsters) is not a Techno object, so the object picker admits it only through this flag. A click over the monster lands on it rather than passing to the ground beneath. Setting the flag also rewrites two of the type's own settings once its section has been read: the type stops being sentient and becomes a legal target, overriding [`LegalTarget=no`](/keys/legaltarget/) in the same section.

The flag does not select which type a monster is built from; [`VeinholeTypeClass`](/keys/veinholetypeclass/) does that, and it is read whether or not the named type sets this flag.
