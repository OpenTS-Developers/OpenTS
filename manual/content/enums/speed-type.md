---
enum_id: SpeedType
slug: speed-type
title: Locomotion speed type
summary: Movement-table classes used to select terrain speed behavior.
representation: token
bindings:
  key_value_types: [SpeedType]
  scripting_parameter_types: []
source_files: [code/speed.hh, code/const.cpp]
values:
  - { constant: SPEED_FOOT, value: 0, input: "Foot", meaning: "Bipedal foot movement." }
  - { constant: SPEED_TRACK, value: 1, input: "Track", meaning: "Tracked vehicle movement." }
  - { constant: SPEED_WHEEL, value: 2, input: "Wheel", meaning: "Wheeled vehicle movement." }
  - { constant: SPEED_HOVER, value: 3, input: "Hover", meaning: "Hover movement." }
  - { constant: SPEED_WINGED, value: 4, input: "Winged", meaning: "Aircraft movement." }
  - { constant: SPEED_FLOAT, value: 5, input: "Float", meaning: "Watercraft movement." }
  - { constant: SPEED_AMPHIBIOUS, value: 6, input: "Amphibious", meaning: "Amphibious movement." }
  - { constant: SPEED_CREEP, value: 7, input: "Creep", meaning: "Slow creeping movement." }
---

A SpeedType selects the column of the [terrain movement table](/systems/movement-and-terrain/) that a type is costed against. One rules.ini section per [land type](/reference/enums/land-type/) holds that table: `[Clear]`, `[Road]`, `[Water]` and the rest. Each section has one entry per speed type, named for a token below, plus [`Buildable`](/keys/buildable/), and every entry is a fractional speed multiplier for one land type and one speed type. `1.0` is full speed, and `0` makes the cell impassable rather than slow. A value above `1.0` is clamped down as it is read.

A SpeedType does not choose the locomotion physics, which [`Locomotor`](/keys/locomotor/) sets separately on the same type. Only a vehicle's rules section accepts a [`SpeedType=`](/keys/speedtype/) entry. Infantry types are always `Foot` and aircraft types always `Winged`, and a structure's speed follows its [`WaterBound`](/keys/waterbound/) setting.
