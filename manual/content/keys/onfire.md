---
key: OnFire
summary: The three flames a structure shows when a sparky warhead knocks it down a damage level.
see_also: [Sparky, SmallFire, LargeFire, TreeFire]
when_omitted:
  kind: value
  value: ""
---

```ini title="rules.ini"
[AudioVisual]
OnFire=MYFIRE_SM,MYFIRE_MD,MYFIRE_LG ; AnimTypes registered in [Animations]
```

The gameplay path reads exactly three slots and reads them by position: the first two are the common flames and the third is the rare one. [`Sparky`](/keys/sparky/) covers the roll that selects between them and what an incomplete list does there. Entries past the third are never reached by it, so a longer list only wastes names.

The rest of the game's fires come from elsewhere. [`SmallFire`](/keys/smallfire/) and [`LargeFire`](/keys/largefire/) cover the flames a structure shows without a sparky warhead and while it is being destroyed, and [`TreeFire`](/keys/treefire/) covers a burning terrain object.

:::danger[Fewer than three entries makes the sparky path read past the list]
Nothing checks the list length before the three slots are taken, and the list starts empty before the rules fill it. [`Sparky`](/keys/sparky/) states the outcome in full: the game either crashes or creates an animation from whatever the missing slot happened to point at.
:::
