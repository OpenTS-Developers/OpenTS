---
key: EnemyHealth
summary: Flag intended to hide an enemy object's condition bar.
no_effect: true
see_also: [ConditionYellow, ConditionRed, Selectable]
when_omitted:
  kind: value
  value: "yes"
---

The name promises control over whether an enemy object shows a condition bar. One test remains. The decorations drawn for a selected or sensor-detected structure or [core defender](/keys/iscoredefender/#scope-buildingtype) check the flag alongside whether the object's house is allied to the player. Sensor-detected means a submerged object in a cell inside a [sensor array's](/keys/sensorarray/) detection radius. On either count they call the routine that used to draw the old-style health bar. That routine survives as an empty body and draws nothing.

The condition indicator the game actually draws goes up once that block has closed. That draw covers every selected or sensor-detected object, not only structures and core defenders. It reads neither the flag nor the owner, so such an object shows its condition whoever owns it and whatever the setting says. The object under the mouse pointer shows its condition on the same terms, limited to what the pointer may resolve at all. A structure draws the pip bar it would draw for a selection. Anything else draws bare condition pips, without the select graphic that frames them and boxes the object while it is selected. A second pair of tests on the same flag sits in an older drawing routine that is compiled out.
