---
enum_id: CrateType
slug: crate
title: Crate result
summary: Random crate outcomes referenced by crate-control settings.
representation: token
bindings:
  key_value_types: [cratetype]
  scripting_parameter_types: []
source_files: [code/crate.hh, code/const.cpp, code/rules.cpp]
values:
  - { constant: CRATE_MONEY, value: 0, input: "Money", meaning: "Award money." }
  - { constant: CRATE_UNIT, value: 1, input: "Unit", meaning: "Create a unit." }
  - { constant: CRATE_HEAL_BASE, value: 2, input: "HealBase", meaning: "Restore every object of the collector's house to maximum strength." }
  - { constant: CRATE_CLOAK, value: 3, input: "Cloak", meaning: "Grant cloaking." }
  - { constant: CRATE_EXPLOSION, value: 4, input: "Explosion", meaning: "Detonate an explosion." }
  - { constant: CRATE_NAPALM, value: 5, input: "Napalm", meaning: "Create a napalm blast." }
  - { constant: CRATE_SQUAD, value: 6, input: "Squad", meaning: "Rewritten to Money before the result is applied." }
  - { constant: CRATE_DARKNESS, value: 7, input: "Darkness", meaning: "Hide explored terrain." }
  - { constant: CRATE_REVEAL, value: 8, input: "Reveal", meaning: "Reveal the whole map." }
  - { constant: CRATE_ARMOR, value: 9, input: "Armor", meaning: "Improve armor." }
  - { constant: CRATE_SPEED, value: 10, input: "Speed", meaning: "Improve movement speed." }
  - { constant: CRATE_FIREPOWER, value: 11, input: "Firepower", meaning: "Improve firepower." }
  - { constant: CRATE_ICBM, value: 12, input: "ICBM", meaning: "Grant the house a missile superweapon." }
  - { constant: CRATE_INVULN, value: 13, input: "Invulnerability", meaning: "No result handler; the crate is consumed and its animation plays." }
  - { constant: CRATE_VETERAN, value: 14, input: "Veteran", meaning: "Promote nearby objects." }
  - { constant: CRATE_ION_STORM, value: 15, input: "IonStorm", meaning: "No result handler; the crate is consumed and its animation plays." }
  - { constant: CRATE_GAS, value: 16, input: "Gas", meaning: "Release gas." }
  - { constant: CRATE_TIBERIUM, value: 17, input: "Tiberium", meaning: "Create Tiberium." }
  - { constant: CRATE_POD, value: 18, input: "Pod", meaning: "No result handler; the crate is consumed and its animation plays." }
---

The list is closed, and a mod cannot add a twentieth result. These tokens are the keys of the `[Powerups]` section, which [Crate powerups](/formats/powerups/) covers. [Crates](/systems/crates/) covers how a result is drawn and what each one does.
