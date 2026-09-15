---
enum_id: MeteorShowerType
slug: meteor-shower
title: Meteor shower intensity
summary: Fixed intensity presets used by meteor-shower trigger actions.
representation: integer
bindings:
  key_value_types: []
  scripting_parameter_types: [meteor-shower]
source_files: [code/meteor.hh, code/taction.cpp]
values:
  - { constant: SHOWER_DRIZZLE, value: 0, input: "0", meaning: "Light drizzle preset." }
  - { constant: SHOWER_SHOWER, value: 1, input: "1", meaning: "Standard shower preset." }
  - { constant: SHOWER_DOWNPOUR, value: 2, input: "2", meaning: "Heavy downpour preset." }
  - { constant: SHOWER_ARMAGEDDON, value: 3, input: "3", meaning: "Maximum Armageddon preset." }
---

The presets differ in how many meteors fall and how widely they scatter, and in nothing else. A drizzle drops one meteor, a shower five, a downpour nine and an Armageddon fifteen, and each of those counts takes up to two more added at random. The area they land over widens with the count. What an individual meteor does where it comes down belongs to the `METLARGE` and `METSMALL` animations the engine picks between. Raising the preset covers more ground with more meteors, and none of them strikes any harder.

:::caution[There is no fifth intensity]
The four counts are the whole of this domain and the engine holds no count for a position beyond them. A fifth setting or higher, or a negative one, therefore takes its meteor count from outside the table, and [Meteor Shower At](/mapping/actions/taction-meteor-shower/) covers what falls out of that.
:::
