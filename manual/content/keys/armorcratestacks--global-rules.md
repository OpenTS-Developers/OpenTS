---
key: ArmorCrateStacks
scope: global-rules
label: Let armor crates stack
summary: Lets an armor crate upgrade an object that an earlier armor crate already upgraded.
see_also: [FirepowerCrateStacks, "system:crates"]
when_omitted:
  kind: value
  value: "no"
---

With `ArmorCrateStacks=yes`, each armor crate multiplies the armor multiplier of every object in [`CrateRadius`](/keys/crateradius/) by the `Armor` value in `[Powerups]` again, so three crates at `2` leave an object taking an eighth of ordinary damage. A drawn armor result also stops turning into money when the collector is already upgraded.

With `no`, an armor crate changes only objects whose armor multiplier is still exactly `1`.
