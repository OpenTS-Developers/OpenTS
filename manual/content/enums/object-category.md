---
enum_id: CategoryType
slug: object-category
title: Object category
summary: Classification tokens set on by infantry, vehicle, aircraft and building types, which only the dropship loadout screen reads.
representation: token
bindings:
  key_value_types: [CategoryType]
  scripting_parameter_types: []
source_files: [code/category.hh, code/category.cpp]
values:
  - { constant: CATEGORY_SOLDIER, value: 0, input: "Soldier", meaning: "Combat soldier." }
  - { constant: CATEGORY_CIVILIAN, value: 1, input: "Civilian", meaning: "Noncombatant civilian." }
  - { constant: CATEGORY_VIP, value: 2, input: "VIP", meaning: "VIP, agent, or commando." }
  - { constant: CATEGORY_RECON, value: 3, input: "Recon", meaning: "Reconnaissance vehicle." }
  - { constant: CATEGORY_AFV, value: 4, input: "AFV", meaning: "Armored fighting vehicle." }
  - { constant: CATEGORY_IFV, value: 5, input: "IFV", meaning: "Infantry fighting vehicle." }
  - { constant: CATEGORY_ARTY, value: 6, input: "LRFS", meaning: "Indirect or long-range fire support." }
  - { constant: CATEGORY_SUPPORT, value: 7, input: "Support", meaning: "Miscellaneous support vehicle." }
  - { constant: CATEGORY_TRANSPORT, value: 8, input: "Transport", meaning: "Ground transport vehicle." }
  - { constant: CATEGORY_AIRSUPPORT, value: 9, input: "AirPower", meaning: "Air combat support." }
  - { constant: CATEGORY_AIRTRANSPORT, value: 10, input: "AirLift", meaning: "Air transport." }
---

Only one of the eleven ever decides anything. The dropship loadout screen leaves an InfantryType marked `Civilian` out of the cameos a player may pick from. It does even that only where the scenario supplies no allowable-units list of its own. Nothing the engine itself calls compares against the other ten.

Each category has two spellings and answers to both, and the comparison ignores case: `recon vehicle` and `Recon` name the same category. A name that is neither leaves the type with no category at all rather than falling back on the previous one. `Soldier` and `Civilian` spell the two forms alike, so the table pairs the remaining nine. Read it in either direction, since a rules file may use the long form where a modder expects the short.

| Short token | Long description |
| --- | --- |
| `VIP` | `VIP/Agent` |
| `Recon` | `Recon Vehicle` |
| `AFV` | `Armored Fighting Vehicle` |
| `IFV` | `Infantry Fighting Vehicle` |
| `LRFS` | `Indirect Fire Support` |
| `Support` | `Misc. Support Vehicle` |
| `Transport` | `Transport Vehicle` |
| `AirPower` | `Air Combat Support` |
| `AirLift` | `Air Transport` |
