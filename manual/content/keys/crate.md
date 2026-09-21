---
key: Crate
summary: Whether the overlay is a crate, which infantry and vehicles collect by entering its cell.
see_also: ["system:crates"]
when_omitted:
  kind: value
  value: "no"
---

Any overlay with `Crate=yes` is a crate. Infantry, walkers, hovercraft and driven vehicles collect it by entering its cell, and the one that entered receives a result. A computer-controlled house's infantry never enter such a cell, and in a campaign its vehicles do not either.

Being a crate does not by itself put an overlay in the campaign result lookup. That lookup recognizes only the overlays named by [`CrateImg`](/keys/crateimg/) and [`WoodCrateImg`](/keys/woodcrateimg/), and of the two the engine places only the `WoodCrateImg` one itself. [Choosing the result](/systems/crates/#choosing-the-result) covers what each of them delivers.
