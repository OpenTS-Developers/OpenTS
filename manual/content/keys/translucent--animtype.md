---
key: Translucent
scope: animtype
label: Animation fade by stage
see_also: ["Translucency", "TranslucencyDetailLevel", "End"]
when_omitted:
  kind: value
  value: "no"
---

The animation thins out as it plays rather than holding one level. It is drawn solid through the first fifth of its stages, a quarter faded past a fifth, half faded past two fifths and three quarters faded past three fifths. It stays there to the end, so the effect reads as something dispersing as it plays out.

The thresholds are fractions of the animation's stage count, so changing [`End=`](/keys/end/) moves all three at once and a short animation reaches its faintest level within a few frames.

The flag takes precedence over [`Translucency=`](/keys/translucency/#scope-animtype), which is not read while it is set.

The fade is applied only at the detail settings the type asks for, which [`TranslucencyDetailLevel`](/keys/translucencydetaillevel/) covers. While it is being applied, an animation belonging to a structure that has faded completely out of sight is not drawn at all.

A [`Tiled=yes`](/keys/tiled/) animation applies the fade onto every copy it draws.
