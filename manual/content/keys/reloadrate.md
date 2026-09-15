---
key: ReloadRate
summary: The interval between the ammunition points a rearming building hands the object docked with it.
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: ".05"
---

```ini title="rules.ini"
[General]
ReloadRate=.1   ; one ammunition point every 90 frames
```

The value is a fraction of a minute, multiplied by 900 frames where it is used, so a docked object gains one point of [`Ammo`](/keys/ammo/) every 45 frames at the default. It is read in one place only: the branch a [`UnitReload=yes`](/keys/unitreload/) building runs. Nothing about weapon reloading between shots, repair timing or aircraft turnaround reads it.
