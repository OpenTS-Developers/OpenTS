---
key: Full
summary: Fills the storage of every member created for a reinforcement group of this type.
see_also: [Storage, VeteranLevel, "system:tiberium"]
when_omitted:
  kind: value
  value: "no"
---

As a reinforcement group builds its members from the TaskForce, each one whose type sets [`Storage`](/keys/storage/) above zero starts with that many units already in its hold. The units are all the first Tiberium type in the rules. A member with no storage is unaffected.

```ini title="ai.ini or map file"
[MyHarvesterDelivery] ; example TeamType
Name=Loaded harvesters
TaskForce=MyHarvesterForce ; defined under [TaskForces]
Script=MyHarvesterScript   ; defined under [ScriptTypes]
Full=yes
```

The value is read at the moment a member is created, alongside [`VeteranLevel`](/keys/veteranlevel/), so it reaches only the paths that build a group from the TaskForce. A team that fills its roster by recruiting objects the house already owns never reads it, and nothing tops a member up again after it has spent the load.
