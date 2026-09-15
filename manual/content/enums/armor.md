---
enum_id: ArmorType
slug: armor
title: Armor
summary: Armor classes used when warheads calculate their effectiveness against a target.
representation: token
bindings:
  key_value_types: [ArmorType]
  scripting_parameter_types: []
source_files: [code/armor.hh, code/const.cpp, code/weapon.cpp]
values:
  - { constant: ARMOR_NONE, value: 0, input: "none", meaning: "Unarmored target class." }
  - { constant: ARMOR_WOOD, value: 1, input: "wood", meaning: "Wood and other light structural armor." }
  - { constant: ARMOR_ALUMINUM, value: 2, input: "light", meaning: "Light vehicle armor." }
  - { constant: ARMOR_STEEL, value: 3, input: "heavy", meaning: "Heavy vehicle armor." }
  - { constant: ARMOR_CONCRETE, value: 4, input: "concrete", meaning: "Concrete building armor." }
---

Armor names are matched without regard to case, and the five classes are fixed by the engine. What each class does with incoming damage is decided by the firing warhead's [`Verses`](/keys/verses/) list.

:::caution[An unrecognized name resolves to `none`]
`none` is an ordinary class with a column of its own rather than an error value. A misspelled armor name therefore does not fail; it quietly moves the type onto the first entry of every warhead's list.
:::
