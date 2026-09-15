---
key: WallBuildSpeedCoefficient
summary: The multiplier applied to a wall's build time.
when_omitted:
  kind: value
  value: ".5"
---

A BuildingType marked [`Wall=yes`](/keys/wall/#scope-buildingtype) has its build time multiplied by this value, applied last of everything that shapes production, so at the default a wall is built in half the time its cost would otherwise buy. No other object type reads it.
