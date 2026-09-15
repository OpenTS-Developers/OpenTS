---
key: Wood
summary: Lets the warhead damage terrain objects, and wall overlays whose armor is wood.
see_also: ["system:walls-and-gates", "Wall", "Armor"]
when_omitted:
  kind: value
  value: "no"
---

A TerrainType takes damage from this warhead and from no other. Damage processing on a terrain object begins by testing the flag and reports nothing at all when it is absent, so a warhead left at `no` cannot fell a tree however much damage it deals. A vehicle likewise treats a terrain object in its path as destroyable only when the weapon it would use against it sets the flag and the terrain type is not [`Immune=yes`](/keys/immune/#scope-aircrafttype), and as impassable otherwise.

The same flag is the second route through a wall. An explosion reduces a wall overlay either because its warhead is [`Wall=yes`](/keys/wall/#scope-warheadtype) or because it is `Wood=yes` and the overlay's [`Armor`](/keys/armor/#scope-aircrafttype) is `wood`. A vehicle's blocked-path test and the player's attack cursor over a hostile wall accept the same pair of conditions. The [computer's automatic wall scan](/systems/target-selection/#what-each-kind-of-object-considers) and infantry's blocked-path test read `Wall=yes` alone. The computer therefore never picks a wood-armored wall out as something to shoot, and infantry treat it as flatly impassable rather than as something to clear.

[`Tiberium=yes`](/keys/tiberium/#scope-overlaytype) sets an overlay's armor to wood as its section is read, which is what puts Tiberium overlays on the same footing.
