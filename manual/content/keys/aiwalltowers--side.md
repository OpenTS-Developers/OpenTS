---
key: AIWallTowers
scope: side
label: Side wall towers
see_also: [WallTower, AIWallDefense, AIWallDefenseCoefficient, "system:ai-base-building"]
when_omitted:
  kind: computed
  note: The first side takes WallTower as each rules file sets it; every other side lists none.
---

```ini title="rules.ini"
[GDI]
AIWallTowers=GACTWR
```

The BuildingTypes a computer house playing for this side lays its base defenses on. The first entry the house's country may own is planned ahead of every `-1` defense placeholder, the perimeter wall gains pairs of it and a placeholder up to the cap [`AIWallDefense`](/keys/aiwalldefense/#scope-side) and [`AIWallDefenseCoefficient`](/keys/aiwalldefensecoefficient/#scope-side) set, and the wall cells become the [threat ring](/systems/ai-base-building/#base-defenses) the defense planner draws from. Every listed type is also treated as already owned when the planner tests a defense's prerequisites. A side whose list names nothing the country may own plans plain placeholders and no ring.

A tower node is armed with one of the defenses that [plug into](/keys/powersupbuilding/) it, written into the placeholder behind it. A tower none of the country's defenses plug into is not built: its node takes a standalone defense instead, so a side is never left with bare towers, but it gains nothing from naming a tower whose upgrades its country cannot own.
