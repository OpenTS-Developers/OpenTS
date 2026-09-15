---
key: StartPitch
summary: The barrel elevation a structure is created with, in eighths of a turn, where 2 is level.
see_also: [StartFacing, IsJuggernaut, Artillary, TickTank, UndeploysInto]
when_omitted:
  kind: value
  value: "2"
---

```ini title="rules.ini"
[DJUGG] ; the stock deployed Juggernaut
IsJuggernaut=yes
StartPitch=2 ; level
```

The figure is read on the same eighth-of-a-turn dial as [`StartFacing`](/keys/startfacing/), applied to barrel elevation instead of body facing, and `2` is level. A structure sets it as its desired barrel elevation as it is created and the barrel swings to it.

A structure that appears by deploying a vehicle is treated differently when its type is [`Artillary=yes`](/keys/artillary/) or [`TickTank=yes`](/keys/ticktank/): its barrel is snapped level at that moment whatever this key says.

Going back the other way, an [`IsJuggernaut=yes`](/keys/isjuggernaut/) structure will not begin to fold away until its barrel is back at this elevation and its body is back at `StartFacing`. An `Artillary=yes` structure takes the same paired test at the point its [`UndeploysInto`](/keys/undeploysinto/) vehicle would be created. Either way the vehicle that results is given this elevation as it appears, so the barrel does not jump between the two forms.

Nothing else reads the figure. A deployed artillery piece that has stopped firing is not returned to it while it idles.
