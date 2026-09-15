---
key: Translucency
scope: animtype
label: Animation fade
see_also: ["Translucent", "TranslucencyDetailLevel", "DetailLevel"]
when_omitted:
  kind: value
  value: "0"
---

Three figures are recognized, and each has to be written exactly: `25` draws the animation a quarter faded, `50` a half and `75` three quarters. Every other number draws it solid, `0` among them; there is no range around the three, so `76` is as solid as `0`.

The figure is read only when [`Translucent=no`](/keys/translucent/#scope-animtype). With that flag set the animation fades by stage instead and this figure is not read at all.

The fade is applied only at the detail settings the type asks for, which [`TranslucencyDetailLevel`](/keys/translucencydetaillevel/) covers.

A structure fading out of sight passes its own level down to the animations it is running. One that has faded completely takes them with it: an animation belonging to a structure that is no longer drawn is not drawn either, whatever this figure says. Short of that, a stated figure replaces the structure's fade rather than adding to it. The animation holds this one level throughout, while an animation that states nothing follows the structure down in steps.

```ini title="art.ini"
[MYBANG16] ; a blast ring, drawn half faded at any detail setting
Image=MYBANG16
Translucency=50
UseNormalLight=yes
Report=EXPNEW11
```
