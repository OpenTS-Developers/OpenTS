---
key: FireSupress
summary: How far around a candidate target allied buildings are searched for when the firing weapon suppresses friendly fire.
see_also: ["system:target-selection"]
when_omitted:
  kind: value
  value: "1"
  note: One cell leaves no ring for the search to walk, so the penalty never applies until the value is raised.
---

The search runs only for an object whose first weapon slot sets [`Supress=yes`](/keys/supress/), and only while that object is scoring candidates. It walks the square cell rings around the candidate from radius `1` out to one cell short of this distance. The candidate's threat score is halved once for every ring cell holding an allied building: four such cells leave a sixteenth of the original score. A building spanning several ring cells counts once per cell.

The value is a distance in cells, and fractions are accepted. The search truncates it to whole cells: `2` walks one ring, `3` walks two, and anything below `2` walks none.

```ini title="rules.ini"
[CombatDamage]
FireSupress=3
```
