---
key: TiberiumSpawnType
summary: The overlay a landing animation plants its Tiberium from.
see_also: ["IsTiberium", "TiberiumSpreadRadius", "system:tiberium"]
when_omitted:
  kind: value
  value: none
---

Each cell the growth reaches gets one of four consecutive overlay types, drawn at random from the one named here and the three that follow it in the rules' overlay list. A patch therefore comes out uneven rather than uniform. The cell is then set to one of the first three growth stages, also at random, and left to grow on its own from there.

The setting is read only where the animation also declares [`IsTiberium=yes`](/keys/istiberium/#scope-animtype) and is thrown by [`Bouncer=yes`](/keys/bouncer/) or [`IsMeteor=yes`](/keys/ismeteor/#scope-animtype). Without a type named here, an animation with all of that still plants nothing, however far [`TiberiumSpreadRadius`](/keys/tiberiumspreadradius/) reaches. The shipped small meteor is in exactly that position: it sets the Tiberium flag and names no type, so the Tiberium a meteor strike leaves behind comes from the debris it spawns rather than from the meteor itself.

```ini title="art.ini"
[METDEBRI]              ; the chunks a meteor breaks into
Bouncer=yes
IsTiberium=true
TiberiumSpawnType=TIB01 ; the first of the green Tiberium overlays
```

The shipped section spells that flag `true` where the prose above says `yes`; the read looks only at the value's first character, so the two spell the same setting. The fragment above leaves out the rest of that section, among it the [`LoopCount=-1`](/keys/loopcount/) a thrown animation needs to keep looping until it lands.

:::danger[A type near the end of the overlay list crashes the game]
The three overlays that follow the named one are taken by position with no check that they exist. Naming one of the last three entries of the rules' overlay list (`CRATE`, `VEINHOLEDUMMY` or `TROCK05` in the shipped rules) lets the pick run off the end. The game reads a pointer that was never an overlay type and faults as the growth is placed.
:::
