---
enum_id: ScrollSpeedType
slug: camera-speed
title: Camera speed
summary: Discrete camera-scroll speeds accepted by camera movement actions.
representation: integer
bindings:
  key_value_types: []
  scripting_parameter_types: [camera-speed]
source_files: [code/scrspeed.hh, code/tactical.cpp]
values:
  - { constant: SCROLL_SPEED_0, value: 0, input: "0", meaning: "The slowest pan: about 667 frames to arrive." }
  - { constant: SCROLL_SPEED_1, value: 1, input: "1", meaning: "About 334 frames to arrive." }
  - { constant: SCROLL_SPEED_2, value: 2, input: "2", meaning: "About 134 frames to arrive." }
  - { constant: SCROLL_SPEED_3, value: 3, input: "3", meaning: "About 34 frames to arrive." }
  - { constant: SCROLL_SPEED_4, value: 4, input: "4", meaning: "The fastest pan: about 17 frames to arrive." }
---

A scripted pan moves the view an equal distance each frame. The setting therefore fixes how long the view takes to arrive rather than how fast it travels, whatever the distance. There is no easing at either end. The five rates are built into the engine and no rules setting reaches them.

:::caution[There is no sixth rate]
A sixth setting or higher, or a negative one, takes its pan rate from outside the table, where nothing fixes how long the pan takes. Both a [scripted camera move](/mapping/actions/taction-center-viewpoint/) and the [team script mission](/mapping/missions/tmission-center-viewpoint/) reach the same rate the same way.
:::
