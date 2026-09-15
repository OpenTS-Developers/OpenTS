---
key: Crushable
scope: animtype
label: Crushable by vehicles
no_effect: true
see_also: [CrushSound, Strength, Crusher]
when_omitted:
  kind: value
  value: "no"
---

An animation is never entered into a cell's occupier list. An AnimType is not a footprint type, so placing one down registers nothing with the cell it sits over. Thirteen tests read the flag: five are fed from that list, four directly and one through the object under the cursor. Six read the cell's overlay. The remaining two are handed a vehicle's attacker or its target. An animation can be none of these, so no gameplay path reads the flag for an animation.

The setting is read out of `art.ini` rather than `rules.ini`, because an AnimType takes all of the settings it shares with other object types from the art database.
