---
key: Meteorites
summary: Parsed flag that schedules nothing.
no_effect: true
see_also: ["system:tiberium"]
when_omitted:
  kind: value
  value: "no"
  note: Each scenario starts with this built-in default.
---

Nothing in the game reads this flag, and setting it makes no meteors fall. Meteors come from the [Meteor Impact At](/mapping/actions/taction-meteor-impact/) and [Meteor Shower At](/mapping/actions/taction-meteor-shower/) trigger actions, and from any animation with [`IsMeteor=yes`](/keys/ismeteor/#scope-animtype) wherever the game creates it.
