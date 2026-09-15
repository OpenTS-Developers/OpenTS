---
key: Sparky
summary: Buildings knocked down a damage level burn with the fire set, and terrain objects struck catch fire.
see_also: [OnFire, SmallFire, Wood, Fire]
when_omitted:
  kind: value
  value: "no"
---

A hit that moves a building down a damage level runs over every cell of its footprint and gives each one a chance of a flame animation attached to the building. The roll is made against a range that widens with the footprint (five plus the building's width plus its height), while the results that produce a flame stay fixed. A structure whose width and height sum to more than four shows fewer flames per cell than a smaller one.

| Roll | Result |
| --- | --- |
| `1` to `5` | The first entry of the fire set, looping one to three times |
| `6` to `8` | The second entry, looping one to three times |
| `9` | The third entry, once |
| Anything else | No flame in that cell |

The range runs from `0` up to that figure, so the third entry is only ever drawn by a building whose width and height sum to four or more. A one-by-one structure never reaches it.

```ini title="rules.ini"
[MyShellWH] ; example WarheadType
Sparky=yes
```

The setting replaces the ordinary smoke rather than adding to it. A warhead that leaves it at `no` instead gives each cell an even chance of the single small-fire animation, and withholds even that when the damage came from an engineer.

Terrain objects are the second effect: one that takes damage above zero and is not already alight catches fire. That path needs [`Wood=yes`](/keys/wood/) on the warhead, which is what allows it to damage a terrain object at all, and an [`Immune=no`](/keys/immune/) terrain type. A warhead without `Wood=yes` never sets a tree alight however much damage it deals.

:::danger[The fire set is read by position]
The three entries are taken from [`OnFire`](/keys/onfire/) by their slots without checking how many the list holds, and `OnFire` is empty until a rules file sets it. With the list empty the game crashes as soon as one of the flame rolls comes up. For most buildings that is the first knock-down, and never more than a few. With one or two entries the missing slot is read from uninitialized memory and handed straight to the animation system, so the game either crashes or draws an animation chosen by whatever happened to be in that memory. Give `OnFire` three entries before setting this on any warhead.
:::
