---
key: Wall
scope: warheadtype
label: Wall destroyer
see_also: ["system:walls-and-gates", "Wood"]
when_omitted:
  kind: value
  value: "no"
---

An explosion from this warhead [reduces the wall overlay](/systems/walls-and-gates/#taking-damage) in the cell it lands on, passing its own damage figure to the per-hit test; [Verses](/keys/verses/) is never read for a wall. A warhead left at `no` reaches the wall's cell without touching the wall, unless it sets [`Wood=yes`](/keys/wood/) and the overlay's armor is wood.

Two other things read the same flag. A warhead that destroys walls also destroys bridges, in scenarios where bridge destruction is switched on, and it cracks ice.

The flag decides more than damage. A vehicle treats a wall in its path as destroyable instead of impassable when its first weapon's warhead sets this flag, or when that warhead is [`Wood=yes`](/keys/wood/) and the wall's armor is wood. Infantry make the same test with no wood alternative, and the attack cursor over a hostile wall appears on the same condition. The [computer's automatic search for walls to shoot](/systems/target-selection/#what-each-kind-of-object-considers) rejects any object whose primary warhead lacks it, and the computer house's [DestroyWalls](/keys/destroywalls/) difficulty flag can stop the scan weighing walls at all.
