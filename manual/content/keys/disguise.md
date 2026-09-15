---
key: Disguise
summary: The InfantryType whose name and artwork a disguised soldier shows to everyone else.
see_also: ["system:capture", Disguised]
when_omitted:
  kind: value
  value: none
---

Two substitutions run off this one type, and each has its own ownership test. A [`Disguised=yes`](/keys/disguised/) soldier of a house the local player does not control is listed under this type's [`Name=`](/keys/name/) string. A `Disguised=yes` soldier of any house but the local player's own is drawn with this type's shape. Outside a campaign the two tests agree. Inside one they do not, because a second house the scenario marks [`PlayerControl=yes`](/keys/playercontrol/) counts as controlled for the name and not for the artwork. Its disguised soldiers keep their own name while wearing the disguise.

The swap reaches the shape file and nothing else. The soldier keeps its own sequence data and steps through the disguise artwork on frame numbers taken from its real type. The two types therefore need matching sequences for the disguise to animate.

Both reads check for a type first, so with none named a disguised soldier simply shows its own name and its own shape to everyone.
