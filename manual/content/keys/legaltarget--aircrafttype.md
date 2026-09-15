---
key: LegalTarget
scope: aircrafttype
label: Object targetability
see_also: ["system:target-selection"]
when_omitted:
  kind: context-dependent
  note: Most object types start targetable. A BulletType, SmudgeType, TerrainType or VoxelAnimType starts untargetable instead, and a TerrainType is forced back to targetable when its section sets IsVeinhole=yes.
---

A type set to `no` is rejected by every automatic target scan, and pointing at one of its objects offers no attack cursor. Holding the force-fire modifier (left Ctrl by default) bypasses the cursor test, so a player can still order a shot at it by hand. An object that already has such a target keeps engaging it: the flag is weighed while scans pick candidates and while the cursor is decided, never while a shot is being taken. Retaliation runs without a scan, so a unit damaged by such an object still fires back at it.

```ini title="rules.ini"
[MYPROP] ; example BuildingType used as scenery
LegalTarget=no
```

Tree targeting is a separate allowance: with `[CombatDamage] TreeTargeting=yes` a player may point at a TerrainType whatever this setting says.
