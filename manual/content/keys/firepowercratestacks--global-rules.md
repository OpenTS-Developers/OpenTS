---
key: FirepowerCrateStacks
scope: global-rules
label: Let firepower crates stack
summary: Lets a firepower crate upgrade an object that an earlier firepower crate already upgraded.
see_also: [ArmorCrateStacks, "system:crates"]
when_omitted:
  kind: value
  value: "no"
---

With `FirepowerCrateStacks=yes`, each firepower crate multiplies the firepower multiplier of every object in [`CrateRadius`](/keys/crateradius/) by the `Firepower` value in `[Powerups]` again, so two crates at `2` leave an object dealing four times its ordinary damage. A drawn firepower result also stops turning into money when the collector is already upgraded; it still turns into money when the collector has no primary weapon.

With `no`, a firepower crate changes only objects whose firepower multiplier is still exactly `1`.
